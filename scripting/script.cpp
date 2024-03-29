#include "script.h"

#include <v8-script.h>
#include <v8-local-handle.h>
#include <v8-function.h>
#include <v8-value.h>
#include <v8-external.h>
#include <filesystem>

#include "entity.h"
#include "scene.h"

#include "script_context.h"
#include "script_exception.h"
#include "environment_templates.h"

#include "validation.h"
#include "logger.h"


namespace scripting
{

bool Script::LoadSource(std::string resourcePath, Entity* entity)
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
    
    Compile(entity);
    return true;
}

bool Script::LoadSource(
    std::string resourcePath, std::string source, Entity* entity)
{
    this->resourcePath = resourcePath;
    this->source = source;

    return Compile(entity);
}

bool Script::Compile(Entity* entity)
{
    scriptInstance.Reset();
    UnSubscribeAll();
    OnCreatedCalled = false;

    // isolate -> handle -> context scopes
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
        isolate, scriptContext->GetContext());
    v8::Context::Scope contextScope(localContext);

    v8::Local<v8::String> v8Script =
        v8::String::NewFromUtf8(isolate, source.c_str())
        .ToLocalChecked();


    // Input objects needs to store callback handles back to script object
    // Assetmanager needs to have reference to the manager entity's scene points to.
    v8::Local<v8::Object> systemObject = 
        v8::Local<v8::Object>::New(isolate, scriptContext->GetSystemObject());

    v8::Local<v8::Object> inputObject = 
        systemObject->Get(localContext,
            v8::String::NewFromUtf8Literal(isolate, "Input"))
        .ToLocalChecked().As<v8::Object>();
    inputObject->SetInternalField(0, v8::External::New(isolate, this));

    v8::Local<v8::Object> assetManagerObject = 
        systemObject->Get(localContext,
            v8::String::NewFromUtf8Literal(isolate, "AssetManager"))
        .ToLocalChecked().As<v8::Object>();
    assetManagerObject->SetInternalField(0, v8::External::New(isolate, assetManager));


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
        if(tryCatch.HasCaught())
        {
            ExceptionHandler(&tryCatch, isolate);
        }

        scriptInstance.Reset();
        return false;
    }
    else if (!script->Run(localContext).ToLocal(&result))
    {
        if(tryCatch.HasCaught())
        {
            ExceptionHandler(&tryCatch, isolate);
        }

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

        if(tryCatch.HasCaught())
        {
            ExceptionHandler(&tryCatch, isolate);
        }

        scriptInstance.Reset();
        return false;
    }
    else
    {
        prototype = value.As<v8::Function>();
    }
    
    v8::Local<v8::FunctionTemplate> entityTemplate =
        v8::Local<v8::FunctionTemplate>::New(
            isolate, Templates::entityTemplate);

    v8::Local<v8::Function> entityFunction =
        entityTemplate->GetFunction(localContext).ToLocalChecked();
    v8::Local<v8::Object> v8Entity = 
        entityFunction->NewInstance(localContext).ToLocalChecked();
    
    v8Entity->SetInternalField(0, v8::External::New(isolate, entity));
    v8::Local<v8::Value> v8EntityValue = v8Entity.As<v8::Value>();

    v8::Local<v8::Object> localScriptInstance;
    if (!prototype->NewInstance(localContext, 1, &v8EntityValue)
        .ToLocal(&localScriptInstance))
    {
        Logger::Write(
            "[Scripting] Cannot create an instance.",
            Logger::Level::Warning, Logger::MsgType::Scripting);
        
        if(tryCatch.HasCaught())
        {
            ExceptionHandler(&tryCatch, isolate);
        }
        
        scriptInstance.Reset();
        return false;
    }

    scriptInstance.Reset(isolate, localScriptInstance);
    return true;
}

void Script::Run(Timestep ts)
{
    if (scriptInstance.IsEmpty())
    {
        return;
    }

    std::string callbackName;
    if (OnCreatedCalled)
    {
        callbackName = "OnUpdated";
    }
    else
    {
        OnCreatedCalled = true;
        callbackName = "OnCreated";
    }

    RunCallback(callbackName, ts);
}

void Script::RunOnDestroyed()
{
    if (scriptInstance.IsEmpty())
    {
        return;
    }

    RunCallback("OnDestroyed", 0.0f);
}

void Script::RunCallback(std::string callbackName, Timestep ts)
{
    v8::HandleScope scope(isolate);
    v8::Local<v8::Context> localContext =
        v8::Local<v8::Context>::New(isolate, scriptContext->GetContext());
    v8::Context::Scope contextScope(localContext);

    v8::Local<v8::String> v8CallbackName = 
        v8::String::NewFromUtf8(isolate, callbackName.c_str())
        .ToLocalChecked();

    v8::Local<v8::Value> v8Value;
    v8::Local<v8::Object> localScriptInstance =
        v8::Local<v8::Object>::New(isolate, scriptInstance);

    if (!localScriptInstance->Get(localContext, v8CallbackName)
        .ToLocal(&v8Value) || !v8Value->IsFunction())
    {
        Logger::Write(
            "[Scripting] Function <" + callbackName + "> cannot be found",
            Logger::Level::Warning, Logger::MsgType::Scripting
        );

        return;
    }

    // Input objects needs to store callback handles back to script object
    // Assetmanager needs to have reference to the manager entity's scene points to.
    v8::Local<v8::Object> systemObject = 
        v8::Local<v8::Object>::New(isolate, scriptContext->GetSystemObject());

    v8::Local<v8::Object> inputObject = 
        systemObject->Get(localContext,
            v8::String::NewFromUtf8Literal(isolate, "Input"))
        .ToLocalChecked().As<v8::Object>();
    inputObject->SetInternalField(0, v8::External::New(isolate, this));

    v8::Local<v8::Object> assetManagerObject = 
        systemObject->Get(localContext,
            v8::String::NewFromUtf8Literal(isolate, "AssetManager"))
        .ToLocalChecked().As<v8::Object>();
    assetManagerObject->SetInternalField(0, v8::External::New(isolate, assetManager));


    v8::Local<v8::Function> v8Callback = v8Value.As<v8::Function>();

    v8::TryCatch tryCatch(isolate);
    v8::Local<v8::Value> result;
    if (callbackName == "OnUpdated")
    {
        v8::Local<v8::Value> v8Timestep =
            v8::Number::New(isolate, ts).As<v8::Value>();
        
        if (!v8Callback->Call(
            localContext, localScriptInstance, 1, &v8Timestep)
            .ToLocal(&result))
        {
            ASSERT(tryCatch.HasCaught());
            ExceptionHandler(&tryCatch, isolate);

            return;
        }
    }
    else if (callbackName == "OnCreated" || callbackName == "OnDestroyed")
    {
        if (!v8Callback->Call(
            localContext, localScriptInstance, 0, nullptr)
            .ToLocal(&result))
        {
            ASSERT(tryCatch.HasCaught());
            ExceptionHandler(&tryCatch, isolate);

            return;
        }
    }
    else
    {
        throw;
    }
}

int Script::AddEventSubscriber(
    SubscriberContext* subscriberContext,
    std::function<void (Event *)> callback)
{
    subscriberContext->isolate = isolate;
    subscriberContext->scriptContext = scriptContext;

    subscriberContext->handle = EventQueue::GetInstance()->Subscribe(
        EventQueue::InputXR,
        callback
    );

    subscriberContexts.push_back(subscriberContext);
    return subscriberContext->handle;
}

void Script::UnSubscribeAll()
{
    for (auto* context: subscriberContexts)
    {
        EventQueue::GetInstance()->Unsubscribe(context->handle);
        context->function.Reset();
        delete context;
    }

    subscriberContexts.clear();
}

Script::~Script()
{
    scriptInstance.Reset();
    UnSubscribeAll();

    scriptContext = nullptr;
    assetManager = nullptr;
    isolate = nullptr;
}

} // namespace scripting
