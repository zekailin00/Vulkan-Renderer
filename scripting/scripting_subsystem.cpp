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
#include "environment_templates.h"


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
        Templates::ResetAll();

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
        v8::Local<v8::ObjectTemplate>::New(
            GetIsolate(), Templates::globalTemplate);
    v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, globalTemp);

    
    v8::Local<v8::ObjectTemplate> localSystemTemplate =
        v8::Local<v8::ObjectTemplate>::New(GetIsolate(), Templates::systemTemplate);

    v8::Local<v8::Object> systemObject =
        localSystemTemplate->NewInstance(context).ToLocalChecked();

    v8::Local<v8::String> systemStr =
        v8::String::NewFromUtf8Literal(isolate, "System");
    
    context->Global()->Set(context, systemStr, systemObject).FromJust();


    scriptContext->context.Reset(isolate, context);
    scriptContext->isolate = isolate;
    scriptContext->systemObject.Reset(isolate, systemObject);

    scriptContext->ExecuteFromPath("resources/javascript/math.js");
    scriptContext->ExecuteFromPath("resources/javascript/enums.js");
    scriptContext->ExecuteFromPath("resources/javascript/physics.js");

    return scriptContext;
}

void ScriptingSystem::BuildEnvironment()
{
    v8::HandleScope handleScope(isolate);

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = v8::ObjectTemplate::New(isolate);
        Templates::globalTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::FunctionTemplate> temp;
        temp = MakeEntityTemplate(isolate);
        Templates::entityTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::FunctionTemplate> temp;
        temp = MakeSceneTemplate(isolate);
        Templates::sceneTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = MakeMathTemplate(isolate);
        Templates::mathTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = MakeInputTemplate(isolate);
        Templates::inputTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = MakeMeshCompTemplate(isolate);
        Templates::meshCompTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = MakeLightCompTemplate(isolate);
        Templates::lightCompTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = MakeDynamicBodyCompTemplate(isolate);
        Templates::dynamicBodyCompTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = MakeCollsionShapeTemplate(isolate);
        Templates::collisionShapeTemplate.Reset(isolate, temp);
    }

    {
        v8::Local<v8::ObjectTemplate> temp;
        temp = MakeAssetManagerTemplate(isolate);
        Templates::assetManagerTemplate.Reset(isolate, temp);
    }
    
    {
        v8::Local<v8::ObjectTemplate> localSystemTemp =
            v8::ObjectTemplate::New(isolate);
        
        localSystemTemp->Set(isolate, "Print",
            v8::FunctionTemplate::New(isolate, print));

        v8::Local<v8::ObjectTemplate> localMathTemp =
            v8::Local<v8::ObjectTemplate>::New(isolate, Templates::mathTemplate);
        localSystemTemp->Set(isolate, "Math", localMathTemp);

        v8::Local<v8::ObjectTemplate> localInputTemp =
            v8::Local<v8::ObjectTemplate>::New(isolate, Templates::inputTemplate);
        localSystemTemp->Set(isolate, "Input", localInputTemp);

        v8::Local<v8::ObjectTemplate> localAssetManagerTemp =
            v8::Local<v8::ObjectTemplate>::New(
                isolate, Templates::assetManagerTemplate);
        localSystemTemp->Set(isolate, "AssetManager", localAssetManagerTemp);

        Templates::systemTemplate.Reset(isolate, localSystemTemp);
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