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
#include "components/static_body_component.h"
#include "components/dynamic_body_component.h"
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

void SetMeshResourcePath(const v8::FunctionCallbackInfo<v8::Value> &info)
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
            "[Scripting] SetMeshResourcePath parameters are invalid",
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

    if (component->mesh == nullptr)
    {
        Logger::Write(
            "[Scripting] GetMeshResourcePath failed beacuse component has no mesh",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

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

void DynamicRigidbody::AttachShape(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void DynamicRigidbody::DetachShape(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void DynamicRigidbody::GetNbShapes(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void DynamicRigidbody::GetShapes(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void DynamicRigidbody::SetGravity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsBoolean())
    {
        Logger::Write(
            "[Scripting] SetGravity parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    bool isGravity = info[0]->BooleanValue(isolate);
    component->dynamicBody->SetGravity(isGravity);
}

void DynamicRigidbody::GetGravity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    bool isGravity = component->dynamicBody->GetGravity();

    info.GetReturnValue().Set(isGravity);
}

void DynamicRigidbody::WakeUp(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    component->dynamicBody->WakeUp();
}

void DynamicRigidbody::SetDensity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsNumber())
    {
        Logger::Write(
            "[Scripting] SetDensity parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    float density = info[0]->NumberValue(context).ToChecked();
    component->dynamicBody->SetDensity(density);
}

void DynamicRigidbody::GetDensity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    float density = component->dynamicBody->GetDensity();
    info.GetReturnValue().Set(density);
}

void DynamicRigidbody::GetMass(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    float mass = component->dynamicBody->GetMass();
    info.GetReturnValue().Set(mass);
}

void DynamicRigidbody::GetMassSpaceInertiaTensor(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    glm::vec3 inertia = component->dynamicBody->GetMassSpaceInertiaTensor();
    
    v8::Local<v8::Object> v8Inertia = toV8(inertia, isolate);
    info.GetReturnValue().Set(v8Inertia);
}

void DynamicRigidbody::SetLinearDamping(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsNumber())
    {
        Logger::Write(
            "[Scripting] SetLinearDamping parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    float linearDamp = info[0]->NumberValue(context).ToChecked();
    component->dynamicBody->SetLinearDamping(linearDamp);
}

void DynamicRigidbody::GetLinearDamping(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    float linearDamp = component->dynamicBody->GetLinearDamping();
    info.GetReturnValue().Set(linearDamp);
}

void DynamicRigidbody::SetAngularDamping(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsNumber())
    {
        Logger::Write(
            "[Scripting] SetAngularDamping parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    float angularDamp = info[0]->NumberValue(context).ToChecked();
    component->dynamicBody->SetAngularDamping(angularDamp);
}

void DynamicRigidbody::GetAngularDamping(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    float angularDamp = component->dynamicBody->GetAngularDamping();
    info.GetReturnValue().Set(angularDamp);
}

void DynamicRigidbody::AddForce(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] AddForce parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::vec3 force;
    if (!toCpp(force, info[0].As<v8::Object>(), isolate))
    {
        Logger::Write(
            "[Scripting] AddForce parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    component->dynamicBody->AddForce(force);
}

void DynamicRigidbody::AddTorque(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] AddTorque parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::vec3 torque;
    if (!toCpp(torque, info[0].As<v8::Object>(), isolate))
    {
        Logger::Write(
            "[Scripting] AddTorque parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    component->dynamicBody->AddTorque(torque);
}

void DynamicRigidbody::ClearForce(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);
    
    component->dynamicBody->ClearForce();
}

void DynamicRigidbody::ClearTorque(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);
    
    component->dynamicBody->ClearTorque();
}

void DynamicRigidbody::GetLinearVelocity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    glm::vec3 linearVelocity = component->dynamicBody->GetLinearVelocity();
    
    v8::Local<v8::Object> v8LinearVelocity = toV8(linearVelocity, isolate);
    info.GetReturnValue().Set(v8LinearVelocity);
}

void DynamicRigidbody::SetLinearVelocity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] SetLinearVelocity parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::vec3 linearVelocity;
    if (!toCpp(linearVelocity, info[0].As<v8::Object>(), isolate))
    {
        Logger::Write(
            "[Scripting] SetLinearVelocity parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    component->dynamicBody->SetLinearVelocity(linearVelocity);
}

void DynamicRigidbody::GetAngularVelocity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    glm::vec3 angularVelocity = component->dynamicBody->GetAngularVelocity();
    
    v8::Local<v8::Object> v8AngularVelocity = toV8(angularVelocity, isolate);
    info.GetReturnValue().Set(v8AngularVelocity);
}

void DynamicRigidbody::SetAngularVelocity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] SetAngularVelocity parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    glm::vec3 angularVelocity;
    if (!toCpp(angularVelocity, info[0].As<v8::Object>(), isolate))
    {
        Logger::Write(
            "[Scripting] SetAngularVelocity parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    component->dynamicBody->SetAngularVelocity(angularVelocity);
}

void DynamicRigidbody::SetKinematic(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    if (info.Length() != 1 || !info[0]->IsBoolean())
    {
        Logger::Write(
            "[Scripting] SetKinematic parameter is invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    bool isKinematic = info[0]->BooleanValue(isolate);
    component->dynamicBody->SetKinematic(isKinematic);
}

void DynamicRigidbody::GetKinematic(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();

    physics::DynamicBodyComponent* component =
        static_cast<physics::DynamicBodyComponent*>(field->Value());
    ASSERT(component != nullptr);

    bool isKinematic = component->dynamicBody->GetKinematic();

    info.GetReturnValue().Set(isKinematic);
}

} // namespace scripting
