

#include "entity.h"

#include "validation.h"
#include "logger.h"

#include <v8-function.h>
#include <v8-external.h>


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
    
    bool hasComponent = entity->GetComponent((Component::Type)id);
    info.GetReturnValue().Set(hasComponent);
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

