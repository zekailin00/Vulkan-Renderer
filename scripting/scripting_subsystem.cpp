#include "scripting_subsystem.h"

#include <v8-initialization.h>
#include <v8-primitive.h>
#include <v8-context.h>
#include <v8-template.h>
#include <v8-external.h>

#include <string>

#include "logger.h"
#include "component.h"
#include "script_component.h"
#include "environment/environment.h"

namespace scripting
{

void ScriptingSystem::Initialize(char* argv[])
{
    // Initialization
    if (argv)
    {
        v8::V8::InitializeICUDefaultLocation(argv[0]);
        v8::V8::InitializeExternalStartupData(argv[0]);
    }

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
    BuildEnvironment();

    ComponentLocator::SetInitializer(
        Component::Type::Script, ScriptInitializer());
    ComponentLocator::SetDeserializer(
        Component::Type::Script, ScriptDeserializer());
    this->initialized = true;
}

ScriptingSystem::~ScriptingSystem()
{
    if (initialized)
    {
        globalTemplate.Reset();
        systemTemplate.Reset();
        mathTemplate.Reset();
        inputTemplate.Reset();

        entityTemplate.Reset();
        componentTemplate.Reset();
        sceneTemplate.Reset();

        isolate->Dispose();
        v8::V8::Dispose();
        v8::V8::DisposePlatform();
        delete createParams.array_buffer_allocator;
    }
}

ScriptContext* ScriptingSystem::NewContext()
{
    v8::HandleScope handleScope(isolate);

    ScriptContext* scriptContext = new ScriptContext();

    v8::Local<v8::ObjectTemplate> globalTemp =
        v8::Local<v8::ObjectTemplate>::New(GetIsolate(), globalTemplate);
    v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, globalTemp);

    
    v8::Local<v8::ObjectTemplate> localSystemTemplate =
        v8::Local<v8::ObjectTemplate>::New(GetIsolate(), systemTemplate);

    v8::Local<v8::Object> systemObject =
        localSystemTemplate->NewInstance(context).ToLocalChecked();

    v8::Local<v8::String> systemStr =
        v8::String::NewFromUtf8Literal(isolate, "System");
    
    context->Global()->Set(context, systemStr, systemObject).FromJust();


    scriptContext->context.Reset(isolate, context);
    scriptContext->isolate = isolate;
    scriptContext->systemObject.Reset(isolate, systemObject);

    scriptContext->ExecuteFromPath("resources/javascript/math.js");

    return scriptContext;
}

void ScriptingSystem::BuildEnvironment()
{
    v8::HandleScope handleScope(isolate);

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = v8::ObjectTemplate::New(isolate);
        globalTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::FunctionTemplate> temp;
        temp = MakeEntityTemplate(isolate);
        entityTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::FunctionTemplate> temp;
        temp = MakeSceneTemplate(isolate);
        sceneTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = MakeMathTemplate(isolate);
        mathTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = MakeInputTemplate(isolate);
        inputTemplate.Reset(isolate, temp);
    }
    
    {
        v8::Local<v8::ObjectTemplate> localSystemTemp =
            v8::ObjectTemplate::New(isolate);
        
        localSystemTemp->Set(isolate, "Print",
            v8::FunctionTemplate::New(isolate, print));

        v8::Local<v8::ObjectTemplate> localMathTemp =
            v8::Local<v8::ObjectTemplate>::New(isolate, mathTemplate);
        localSystemTemp->Set(isolate, "Math", localMathTemp);

        v8::Local<v8::ObjectTemplate> localInputTemp =
            v8::Local<v8::ObjectTemplate>::New(isolate, inputTemplate);
        localSystemTemp->Set(isolate, "Input", localInputTemp);

        systemTemplate.Reset(isolate, localSystemTemp);
    }
}


std::string ScriptingSystem::scriptResource =
" = function(entity)\n"
"{\n"
"    this.OnCreated = function(){\n\n"
"    }\n\n"
"    this.OnUpdated = function(timestep){\n\n"
"    }\n\n"
"    this.OnDestroyed = function(){\n\n"
"    }\n"
"}";

} // namespace scripting