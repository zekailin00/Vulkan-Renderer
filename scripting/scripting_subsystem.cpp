#include "scripting_subsystem.h"

#include <v8-initialization.h>
#include <v8-primitive.h>
#include <v8-context.h>
#include <v8-exception.h>
#include <v8-isolate.h>
#include <v8-local-handle.h>
#include <v8-script.h>
#include <v8-function.h>
#include <v8-value.h>
#include <v8-template.h>
#include <v8-external.h>

#include <fstream>
#include <iostream> // debug
#include <filesystem>
#include <string>

#include "logger.h"
#include "environment.h"

namespace scripting
{

void ScriptingSubsystem::Initialize(char* argv[])
{
    // Initialization
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    // New isolate
    createParams.array_buffer_allocator = 
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    isolate = v8::Isolate::New(createParams);

    // It seems the code works fine without this
    // if not doing multithreading
    // Put it here as a placeholder
    v8::Isolate::Scope isolateScope(isolate);
    this->initialized = true;
}

ScriptingSubsystem::~ScriptingSubsystem()
{
    if (initialized)
    {
        globalTemplate.Reset();
        systemTemplate.Reset();
        entityTemplate.Reset();
        componentTemplate.Reset();
        entityObject.Reset();

        isolate->Dispose();
        v8::V8::Dispose();
        v8::V8::DisposePlatform();
        delete createParams.array_buffer_allocator;
    }
}

ScriptContext* ScriptingSubsystem::NewContext()
{
    v8::HandleScope handleScope(isolate);

    ScriptContext* scriptContext = new ScriptContext();

    v8::Local<v8::ObjectTemplate> globalT =
        v8::Local<v8::ObjectTemplate>::New(GetIsolate(), globalTemplate);
    v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, globalT);
    scriptContext->context.Reset(isolate, context);

    return scriptContext;
}

v8::MaybeLocal<v8::Object> ScriptingSubsystem::GetScriptObject(std::string path)
{
    // isolate -> handle -> context scopes
    // v8::Isolate::Scope isolateScope(isolate); not needed??
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::Context> localContext =
        v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope contextScope(localContext);

    v8::Local<v8::String> v8Script;
    {
        std::string script = "";
        {
            std::string buffer;
            std::ifstream source;
            source.open(path);
            if (source.good())
            {
                while(std::getline(source, buffer))
                    script = script + buffer + "\n";
                source.close();
            }
            else 
            {
                Logger::Write(
                    "File does not exist.",
                    Logger::Level::Warning, Logger::MsgType::Scripting
                );
                
                return handleScope.EscapeMaybe(v8::MaybeLocal<v8::Object>());
            }
        }

        v8Script = v8::String::NewFromUtf8(isolate, script.c_str()).ToLocalChecked();
    }

    // Catch exceptions from compiling the script
    v8::TryCatch tryCatch(isolate);

    v8::ScriptOrigin origin(isolate, 
        v8::String::NewFromUtf8(isolate, path.c_str()).ToLocalChecked());
    v8::Local<v8::Context> context(isolate->GetCurrentContext());
    v8::Local<v8::Script> script;
    v8::Local<v8::Value> result;

    if (!v8::Script::Compile(context, v8Script, &origin).ToLocal(&script))
    {
        assert(tryCatch.HasCaught());
        ExceptionHandler(isolate, &tryCatch);
        return handleScope.EscapeMaybe(v8::MaybeLocal<v8::Object>());
    }
    else if (!script->Run(context).ToLocal(&result))
    {
        assert(tryCatch.HasCaught());
        ExceptionHandler(isolate, &tryCatch);
        return handleScope.EscapeMaybe(v8::MaybeLocal<v8::Object>());
    }

    // Get the prototype from JS script
    std::string fileName = path.substr(path.find_last_of("/\\")+1);
    fileName = fileName.substr(0, fileName.size()-3); // remove suffix
    v8::Local<v8::String> prototypeName =
        v8::String::NewFromUtf8(isolate, fileName.c_str()).ToLocalChecked();
    v8::Local<v8::Value> value;
    v8::Local<v8::Function> prototype;

    if (!context->Global()->Get(context, prototypeName).ToLocal(&value) ||
        !value->IsFunction())
    {
        Logger::Write(
            "[Scripting] Prototype cannot be found",
            Logger::Level::Warning, Logger::MsgType::Scripting
        );
        return handleScope.EscapeMaybe(v8::MaybeLocal<v8::Object>());
    }
    else prototype = value.As<v8::Function>();

    v8::Local<v8::Object> instance;
    if (!prototype->NewInstance(localContext).ToLocal(&instance))
    {
        Logger::Write(
            "[Scripting] Cannot create an instance.",
            Logger::Level::Warning, Logger::MsgType::Scripting
        );
        return handleScope.EscapeMaybe(v8::MaybeLocal<v8::Object>());
    }

    return handleScope.EscapeMaybe(v8::MaybeLocal<v8::Object>(instance));
}

void ScriptingSubsystem::SetEntityContext(void* field)
{
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Object> localEntityObject
        = v8::Local<v8::Object>::New(isolate, entityObject);
    
    v8::Local<v8::External> ptr = v8::External::New(isolate, field);
    localEntityObject->SetInternalField(0, ptr);
}

bool ScriptingSubsystem::ExecuteScriptCallback(v8::Local<v8::Object> instance,
    std::string callbackName, float* timestep)
{
    // Extract callbacks from the prototype
    // v8::Isolate::Scope isolateScope(isolate); // not needed??
    v8::HandleScope scope(isolate);
    v8::Local<v8::Context> localContext =
        v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope contextScope(localContext);

    v8::Local<v8::String> strCallback = 
        v8::String::NewFromUtf8(isolate, callbackName.c_str()).ToLocalChecked();

    v8::Local<v8::Value> valCallback;

    if (!instance->Get(localContext, strCallback).ToLocal(&valCallback) ||
        !valCallback->IsFunction())
    {
        Logger::Write(
            "[Scripting] Function <"+callbackName+"> cannot be found",
            Logger::Level::Warning, Logger::MsgType::Scripting
        );
        return false;
    }

    v8::Local<v8::Function> functCallback = valCallback.As<v8::Function>();

    v8::TryCatch tryCatch(isolate);
    v8::Local<v8::Value> result;
    if (timestep)
    {
        v8::Local<v8::Value> v8Timestep = v8::Number::New(isolate, *timestep).As<v8::Value>();
        
        if (!functCallback->Call(localContext, localContext->Global(), 1, &v8Timestep)
            .ToLocal(&result))
        {
            assert(tryCatch.HasCaught());
            ExceptionHandler(isolate, &tryCatch);
            return false;
        }
    }
    else
    {
        if (!functCallback->Call(localContext, localContext->Global(), 0, nullptr)
            .ToLocal(&result))
        {
            assert(tryCatch.HasCaught());
            ExceptionHandler(isolate, &tryCatch);
            return false;
        }
    }

    v8::String::Utf8Value instanceName(isolate, instance->GetConstructorName());
    double debugValue = result->NumberValue(localContext).ToChecked(); // not used
    return true;
}



void ScriptingSubsystem::ExceptionHandler(v8::Isolate* isolate, v8::TryCatch* tryCatch){
    v8::HandleScope handleScope(isolate);
    v8::String::Utf8Value exception(isolate, tryCatch->Exception());

    const char* exceptionString = *exception;
    v8::Local<v8::Message> message = tryCatch->Message();

    if (message.IsEmpty())
    {
        Logger::Write(
            exceptionString,
            Logger::Level::Warning, Logger::MsgType::Scripting
        );
    }
    else 
    {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(isolate,
                                    message->GetScriptOrigin().ResourceName());
        v8::Local<v8::Context> context(isolate->GetCurrentContext());
        const char* filenameString = *filename;
        int linenum = message->GetLineNumber(context).FromJust();
        std::string msg = filenameString;
        msg = msg + ":";
        msg = msg + std::to_string(linenum)+": "+exceptionString;
        Logger::Write(msg, Logger::Level::Warning, Logger::MsgType::Scripting);

        // Print line of source code.
        v8::String::Utf8Value sourceline(isolate, 
            message->GetSourceLine(context).ToLocalChecked());
        const char* sourcelineString = *sourceline;
        Logger::Write(sourcelineString,
            Logger::Level::Warning, Logger::MsgType::Scripting
        );

        // Print wavy underline (GetUnderline is deprecated).
        std::string underline = "";
        int start = message->GetStartColumn(context).FromJust();
        for (int i = 0; i < start; i++)
        {
            underline = underline + " ";
        }
        int end = message->GetEndColumn(context).FromJust();
        for (int i = start; i < end; i++)
        {
            underline = underline + "^";
        }
        underline = underline + "\n";
        Logger::Write(underline, Logger::Level::Warning, Logger::MsgType::Scripting);

        v8::Local<v8::Value> stackTraceString;
        if (tryCatch->StackTrace(context).ToLocal(&stackTraceString) &&
            stackTraceString->IsString() &&
            stackTraceString.As<v8::String>()->Length() > 0)
        {
            v8::String::Utf8Value stackTrace(isolate, stackTraceString);
            const char* err = *(stackTrace);
            Logger::Write(err, Logger::Level::Warning, Logger::MsgType::Scripting);
        }
  }
}

std::string ScriptingSubsystem::scriptResource =
"<filename> = function()\n"
"{\n"
"    this.OnCreated = function(){\n\n"
"    }\n\n"
"    this.OnUpdated = function(timestep){\n\n"
"    }\n\n"
"    this.OnDestroyed = function(){\n\n"
"    }\n"
"}";

} // namespace scripting