#include "entity_callbacks.h"

#include "entity.h"

#include "environment/script_math.h"
#include "scripting_subsystem.h"

#include "validation.h"
#include "logger.h"

#include <v8-function.h>
#include <v8-external.h>
#include <v8-primitive.h>
#include <v8-isolate.h>
#include <v8-value.h>
#include <v8-container.h>

#include <list>

namespace scripting
{

void AddComponent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);
    
    if (info.Length() != 1 || !info[0]->IsInt32())
    {
        Logger::Write(
            "[Scripting] AddComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }
    
    v8::Local<v8::Int32> componentType = info[0].As<v8::Int32>();
    uint32_t id = componentType->Value();

    if(id >= (uint32_t)Component::Type::Size)
    {
        Logger::Write(
            "[Scripting] AddComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    entity->AddComponent((Component::Type)id);

    //TODO: return component
}

void RemoveComponent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);
    
    if (info.Length() != 1 || !info[0]->IsInt32())
    {
        Logger::Write(
            "[Scripting] RemoveComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }
    
    v8::Local<v8::Int32> componentType = info[0].As<v8::Int32>();
    uint32_t id = componentType->Value();

    if(id >= (uint32_t)Component::Type::Size)
    {
        Logger::Write(
            "[Scripting] RemoveComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    entity->RemoveComponent((Component::Type)id);
}

void HasComponent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);
    
    if (info.Length() != 1 || !info[0]->IsInt32())
    {
        Logger::Write(
            "[Scripting] HasComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }
    
    v8::Local<v8::Int32> componentType = info[0].As<v8::Int32>();
    uint32_t id = componentType->Value();

    if(id >= (uint32_t)Component::Type::Size)
    {
        Logger::Write(
            "[Scripting] HasComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    bool hasComponent = entity->HasComponent((Component::Type)id);
    info.GetReturnValue().Set(hasComponent);
}


void GetComponent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);
    
    if (info.Length() != 1 || !info[0]->IsInt32())
    {
        Logger::Write(
            "[Scripting] GetComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }
    
    v8::Local<v8::Int32> componentType = info[0].As<v8::Int32>();
    uint32_t id = componentType->Value();

    if(id >= (uint32_t)Component::Type::Size)
    {
        Logger::Write(
            "[Scripting] GetComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    // TODO: 
}

void ReparentTo(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);
    
    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] ReparentTo parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }
    
    v8::Local<v8::Object> v8Parent = info[0].As<v8::Object>();
    v8::Local<v8::External> field2 =
        v8Parent->GetInternalField(0).As<v8::External>();
    Entity* parent = static_cast<Entity*>(field2->Value());
    ASSERT(entity != nullptr);
    
    entity->ReparentTo(parent);
}

void GetGlobalTransform(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    const glm::mat4& transform = entity->GetGlobalTransform();

    v8::Local<v8::Object> v8Mat4 = toV8(transform, info);
    info.GetReturnValue().Set(v8Mat4);
}

void GetLocalTransform(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    const glm::mat4& transform = entity->GetLocalTransform();

    v8::Local<v8::Object> v8Mat4 = toV8(transform, info);
    info.GetReturnValue().Set(v8Mat4);
}

void SetLocalTransform1(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);
    
    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] SetLocalTransform1 parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

    glm::mat4 transform;

    bool result = toCpp(transform, 
        info[0]->ToObject(context).ToLocalChecked(), info);
    
    if (result)
    {
        entity->SetLocalTransform(transform);
    }
    else
    {
        Logger::Write(
            "[Scripting] SetLocalTransform1 parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
    }
}

void SetLocalTransform2(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);
    
    if (info.Length() != 3 || !info[0]->IsObject() ||
        !info[1]->IsObject() || !info[2]->IsObject())
    {
        Logger::Write(
            "[Scripting] SetLocalTransform2 parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

    glm::vec3 postion;
    glm::vec3 rotation;
    glm::vec3 scale;

    bool result = true;

    result = result && toCpp(postion,
        info[0]->ToObject(context).ToLocalChecked(), info);
    result = result && toCpp(rotation,
        info[1]->ToObject(context).ToLocalChecked(), info);
    result = result && toCpp(scale,
        info[2]->ToObject(context).ToLocalChecked(), info);

    if (result)
    {
        entity->SetLocalTransform(postion, rotation, scale);
    }
    else
    {
        Logger::Write(
            "[Scripting] SetLocalTransform2 parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
    }
}

void GetLocalTranslation(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    glm::vec3 translation = entity->GetLocalTranslation();

    v8::Local<v8::Object> v8Translation = toV8(translation, info);
    info.GetReturnValue().Set(v8Translation);
}

void GetLocalRotation(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    glm::vec3 rotation = entity->GetLocalRotation();

    v8::Local<v8::Object> v8Rotation = toV8(rotation, info);
    info.GetReturnValue().Set(v8Rotation);
}

void GetLocalScale(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::HandleScope handleScope(info.GetIsolate());

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    glm::vec3 scale = entity->GetLocalScale();

    v8::Local<v8::Object> v8Scale = toV8(scale, info);
    info.GetReturnValue().Set(v8Scale);
}

void GetParent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    Entity* parent = entity->GetParent();

    if (parent == nullptr)
    {
        return;
    }

    ScriptingSystem* scriptingSystem = ScriptingSystem::GetInstance();
    v8::Local<v8::FunctionTemplate> entityTemplate =
        v8::Local<v8::FunctionTemplate>::New(isolate,
        scriptingSystem->GetEntityTemplate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Function> entityFunction =
        entityTemplate->GetFunction(context).ToLocalChecked();
    v8::Local<v8::Object> v8Parent = 
        entityFunction->NewInstance(context).ToLocalChecked();
    
    v8Parent->SetInternalField(0, v8::External::New(isolate, parent));
    info.GetReturnValue().Set(v8Parent);
}

void GetChildByName(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    if (info.Length() != 1 || !info[0]->IsString())
    {
        Logger::Write(
            "[Scripting] GetChildByName parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::String::Utf8Value v8Name(
        isolate, info[0]->ToString(context).ToLocalChecked());
    Entity* child = entity->GetChildByName(*v8Name);

    if (child == nullptr)
    {
        return;
    }

    ScriptingSystem* scriptingSystem = ScriptingSystem::GetInstance();
    v8::Local<v8::FunctionTemplate> entityTemplate =
        v8::Local<v8::FunctionTemplate>::New(isolate,
        scriptingSystem->GetEntityTemplate());

    v8::Local<v8::Function> entityFunction =
        entityTemplate->GetFunction(context).ToLocalChecked();
    v8::Local<v8::Object> v8Child = 
        entityFunction->NewInstance(context).ToLocalChecked();
    
    v8Child->SetInternalField(0, v8::External::New(isolate, child));
    info.GetReturnValue().Set(v8Child);
}

void GetChildren(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    ScriptingSystem* scriptingSystem = ScriptingSystem::GetInstance();
    v8::Local<v8::FunctionTemplate> entityTemplate =
        v8::Local<v8::FunctionTemplate>::New(isolate,
        scriptingSystem->GetEntityTemplate());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    v8::Local<v8::Function> entityFunction =
        entityTemplate->GetFunction(context).ToLocalChecked();


    const std::list<Entity*>& children = entity->GetChildren();
    v8::Local<v8::Array> v8Children = v8::Array::New(isolate, children.size());

    int i = 0;
    for (Entity* child: children)
    {
        v8::Local<v8::Object> v8Entity =
            entityFunction->NewInstance(context).ToLocalChecked();

        v8::Local<v8::External> field =
            v8::External::New(info.GetIsolate(), child);
        v8Entity->SetInternalField(0, field);

        v8Children->Set(context, i, v8Entity).ToChecked();
        i++;
    }

    info.GetReturnValue().Set(v8Children);

    /**
     * list = entity.GetChildren()
     * list.length
     * Array.isArray(list) == true
     * list[0].GetChildren()
    */
}

void GetName(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    std::string name = entity->GetName();

    v8::Local<v8::String> v8Name =
        v8::String::NewFromUtf8(isolate, name.c_str())
        .ToLocalChecked();
    
    info.GetReturnValue().Set(v8Name);
}

void SetName(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    if (info.Length() != 1 || !info[0]->IsString())
    {
        Logger::Write(
            "[Scripting] SetName parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::String::Utf8Value v8Name(
        isolate, info[0]->ToString(context).ToLocalChecked());
    
    entity->SetName(*v8Name);
}

void GetScene(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field->Value());
    ASSERT(entity != nullptr);

    Scene* scene = entity->GetScene();

    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    ScriptingSystem* scriptingSystem = ScriptingSystem::GetInstance();
    v8::Local<v8::FunctionTemplate> sceneTemplate =
        v8::Local<v8::FunctionTemplate>::New(isolate,
        scriptingSystem->GetSceneTemplate());

    v8::Local<v8::Function> sceneFunction =
        sceneTemplate->GetFunction(context).ToLocalChecked();
    v8::Local<v8::Object> v8Scene = 
        sceneFunction->NewInstance(context).ToLocalChecked();
    
    v8Scene->SetInternalField(0, v8::External::New(isolate, scene));
    info.GetReturnValue().Set(v8Scene);
}

} // namespace scripting