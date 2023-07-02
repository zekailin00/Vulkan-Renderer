#include "component_callbacks.h"

#include <v8-external.h>
#include <v8-object.h>
#include <v8-template.h>
#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-primitive.h>

#include "entity.h"
#include "scene.h"
#include "component.h"
#include "mesh_component.h"
#include "light_component.h"
#include "renderer_asset_manager.h"

#include "scripting_subsystem.h"
#include "script_math.h"

#include "logger.h"
#include "validation.h"

#include <memory>


namespace scripting
{

void GetEntity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Component* component = static_cast<Component*>(field->Value());
    ASSERT(component != nullptr);

    Entity* entity = component->entity;

    ScriptingSystem* scriptingSystem = ScriptingSystem::GetInstance();
    v8::Local<v8::FunctionTemplate> entityTemplate =
        v8::Local<v8::FunctionTemplate>::New(isolate,
        scriptingSystem->GetEntityTemplate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Function> entityFunction =
        entityTemplate->GetFunction(context).ToLocalChecked();
    v8::Local<v8::Object> v8Entity = 
        entityFunction->NewInstance(context).ToLocalChecked();
    
    v8Entity->SetInternalField(0, v8::External::New(isolate, entity));
    info.GetReturnValue().Set(v8Entity);
}

void GetComponentType(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Component* component = static_cast<Component*>(field->Value());
    ASSERT(component != nullptr);

    int componentType = (int)component->type;
    info.GetReturnValue().Set(componentType);
}

void SetMeshResoucePath(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    renderer::MeshComponent* component =
        static_cast<renderer::MeshComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsString())
    {
        Logger::Write(
            "[Scripting] SetMeshResoucePath parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::String::Utf8Value v8Path(isolate, info[0].As<v8::String>());
    std::string resourcePath = *v8Path;

    renderer::IRendererAssetManager* assetManager =
        dynamic_cast<renderer::IRendererAssetManager*>(
            component->entity->GetScene()->GetAssetManager());
    
    component->mesh = std::dynamic_pointer_cast<renderer::VulkanMesh>(
        assetManager->GetMesh(resourcePath));
}

void GetMeshResourcePath(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    renderer::MeshComponent* component =
        static_cast<renderer::MeshComponent*>(field->Value());
    ASSERT(component != nullptr);

    std::string pathStr = component->mesh->GetResourcePath();

    v8::Local<v8::String> v8Path =
        v8::String::NewFromUtf8(isolate, pathStr.c_str()).ToLocalChecked();

    info.GetReturnValue().Set(v8Path);
}

void SetLightColor(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    renderer::LightComponent* component =
        static_cast<renderer::LightComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] SetLightColor parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::vec3 color;
    if (!toCpp(color, info[0].As<v8::Object>(), isolate))
    {
        Logger::Write(
            "[Scripting] SetLightColor parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    color = glm::clamp(color,
        glm::vec3(0.0, 0.0, 0.0),
        glm::vec3(1000, 1000, 1000)
    );
    component->properties.color = color;
}

} // namespace scripting
