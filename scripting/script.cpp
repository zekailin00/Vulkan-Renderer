#include "script.h"

#include <v8-script.h>
#include <v8-local-handle.h>
#include <v8-function.h>
#include <v8-value.h>
#include <filesystem>

#include "script_context.h"
#include "script_exception.h"

#include "validation.h"
#include "logger.h"


namespace scripting
{

bool Script::LoadSource(std::string resourcePath)
{
    bool result;

    this->resourcePath = resourcePath;
    result = assetManager->GetSourceCode(resourcePath, source);

    if (!result)
    {
        Logger::Write(
            "[Scripting] Loading source code from file system failed.",
            Logger::Level::Warning, Logger::MsgType::Scripting
        );

        resourcePath = "None";
        source = "";
        return false;
    }
    
    return Compile();
}

bool Script::LoadSource(std::string resourcePath, std::string source)
{
    this->resourcePath = resourcePath;
    this->source = source;

    return Compile();
}

bool Script::Compile()
{
    scriptInstance.Reset();

    // isolate -> handle -> context scopes
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
        isolate, scriptContext->GetContext());
    v8::Context::Scope contextScope(localContext);

    v8::Local<v8::String> v8Script =
        v8::String::NewFromUtf8(isolate, source.c_str())
        .ToLocalChecked();

    // Catch exceptions from compiling the script
    v8::TryCatch tryCatch(isolate);

    v8::ScriptOrigin origin(isolate, 
        v8::String::NewFromUtf8(isolate, resourcePath.c_str())
        .ToLocalChecked());

    v8::Local<v8::Script> script;
    v8::Local<v8::Value> result;

    if (!v8::Script::Compile(localContext, v8Script, &origin)
        .ToLocal(&script))
    {
        ASSERT(tryCatch.HasCaught());
        ExceptionHandler(&tryCatch, isolate);

        scriptInstance.Reset();
        return false;
    }
    else if (!script->Run(localContext).ToLocal(&result))
    {
        ASSERT(tryCatch.HasCaught());
        ExceptionHandler(&tryCatch, isolate);

        scriptInstance.Reset();
        return false;
    }

    // Get the prototype from JS script
    std::filesystem::path path = resourcePath;
    std::string fileName = path.stem().string();

    v8::Local<v8::String> prototypeName =
        v8::String::NewFromUtf8(isolate, fileName.c_str()).ToLocalChecked();
    v8::Local<v8::Value> value;
    v8::Local<v8::Function> prototype;

    if (!localContext->Global()
        ->Get(localContext, prototypeName).ToLocal(&value) ||
        !value->IsFunction())
    {
        Logger::Write(
            "[Scripting] Prototype cannot be found",
            Logger::Level::Warning, Logger::MsgType::Scripting);

        scriptInstance.Reset();
        return false;
    }
    else
    {
        prototype = value.As<v8::Function>();
    }

    v8::Local<v8::Object> localScriptInstance;
    if (!prototype->NewInstance(localContext).ToLocal(&localScriptInstance))
    {
        Logger::Write(
            "[Scripting] Cannot create an instance.",
            Logger::Level::Warning, Logger::MsgType::Scripting);
        
        scriptInstance.Reset();
        return false;
    }

    scriptInstance.Reset(isolate, localScriptInstance);
    return true;
}

Script::~Script()
{
    scriptInstance.Reset();

    scriptContext = nullptr;
    assetManager = nullptr;
    isolate = nullptr;
}

} // namespace scripting
