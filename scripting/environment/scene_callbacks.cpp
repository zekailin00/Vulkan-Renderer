#include "scene_callbacks.h"

#include "environment/script_math.h"
#include "environment_templates.h"

#include "scene.h"
#include "entity.h"
#include "validation.h"
#include "logger.h"

#include <v8-function.h>
#include <v8-external.h>
#include <v8-primitive.h>
#include <v8-isolate.h>
#include <v8-value.h>
#include <v8-container.h>

#include <vector>

namespace scripting
{

void GetSceneName(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Scene* scene = static_cast<Scene*>(field->Value());
    ASSERT(scene != nullptr);

    std::string name = scene->GetSceneName();

    v8::Local<v8::String> v8Name =
        v8::String::NewFromUtf8(isolate, name.c_str())
        .ToLocalChecked();
    
    info.GetReturnValue().Set(v8Name);
}

void NewEntity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Scene* scene = static_cast<Scene*>(field->Value());
    ASSERT(scene != nullptr);

    Entity* entity = scene->NewEntity();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::FunctionTemplate> entityTemplate =
        v8::Local<v8::FunctionTemplate>::New(
            isolate, Templates::entityTemplate);

    v8::Local<v8::Function> entityFunction =
        entityTemplate->GetFunction(context).ToLocalChecked();
    v8::Local<v8::Object> v8Entity = 
        entityFunction->NewInstance(context).ToLocalChecked();
    
    v8Entity->SetInternalField(0, v8::External::New(isolate, entity));
    info.GetReturnValue().Set(v8Entity);
}

void RemoveEntity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Scene* scene = static_cast<Scene*>(field->Value());
    ASSERT(scene != nullptr);

    if (info.Length() != 1 || !info[0]->IsObject())
    {
        Logger::Write(
            "[Scripting] RemoveEntity parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::Local<v8::Object> v8Entity = info[0].As<v8::Object>();
    v8::Local<v8::External> field2 =
        v8Entity->GetInternalField(0).As<v8::External>();
    Entity* entity = static_cast<Entity*>(field2->Value());
    ASSERT(entity != nullptr);
    
    scene->RemoveEntity(entity);;
}

void GetEntityByName(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Scene* scene = static_cast<Scene*>(field->Value());
    ASSERT(scene != nullptr);

    if (info.Length() != 1 || !info[0]->IsString())
    {
        Logger::Write(
            "[Scripting] GetEntityByName parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::String::Utf8Value v8Name(
        isolate, info[0]->ToString(context).ToLocalChecked());
    Entity* entity = scene->GetEntityByName(*v8Name);

    if (entity == nullptr)
    {
        return;
    }

    v8::Local<v8::FunctionTemplate> entityTemplate =
        v8::Local<v8::FunctionTemplate>::New(
            isolate, Templates::entityTemplate);

    v8::Local<v8::Function> entityFunction =
        entityTemplate->GetFunction(context).ToLocalChecked();
    v8::Local<v8::Object> v8Entity = 
        entityFunction->NewInstance(context).ToLocalChecked();
    
    v8Entity->SetInternalField(0, v8::External::New(isolate, entity));
    info.GetReturnValue().Set(v8Entity);
}

void GetEntitiesWithComponent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Scene* scene = static_cast<Scene*>(field->Value());
    ASSERT(scene != nullptr);
    
    if (info.Length() != 1 || !info[0]->IsInt32())
    {
        Logger::Write(
            "[Scripting] GetEntitiesWithComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }
    
    v8::Local<v8::Int32> componentType = info[0].As<v8::Int32>();
    uint32_t id = componentType->Value();

    if(id >= (uint32_t)Component::Type::Size)
    {
        Logger::Write(
            "[Scripting] GetEntitiesWithComponent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );
        return;
    }

    v8::Local<v8::FunctionTemplate> entityTemplate =
        v8::Local<v8::FunctionTemplate>::New(
            isolate, Templates::entityTemplate);

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    v8::Local<v8::Function> entityFunction =
        entityTemplate->GetFunction(context).ToLocalChecked();

    std::vector<Entity*> entityList;
    scene->GetEntitiesWithComponent((Component::Type)id, entityList);

    v8::Local<v8::Array> v8EntityList =
        v8::Array::New(isolate, entityList.size());

    int i = 0;
    for (Entity* e: entityList)
    {
        v8::Local<v8::Object> v8Entity =
            entityFunction->NewInstance(context).ToLocalChecked();

        v8::Local<v8::External> field =
            v8::External::New(info.GetIsolate(), e);
        v8Entity->SetInternalField(0, field);

        v8EntityList->Set(context, i, v8Entity).ToChecked();
        i++;
    }

    info.GetReturnValue().Set(v8EntityList);
}

void GetRootEntity(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();

    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Scene* scene = static_cast<Scene*>(field->Value());
    ASSERT(scene != nullptr);

    Entity* entity = scene->GetRootEntity();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::FunctionTemplate> entityTemplate =
        v8::Local<v8::FunctionTemplate>::New(
            isolate, Templates::entityTemplate);

    v8::Local<v8::Function> entityFunction =
        entityTemplate->GetFunction(context).ToLocalChecked();
    v8::Local<v8::Object> v8Entity = 
        entityFunction->NewInstance(context).ToLocalChecked();
    
    v8Entity->SetInternalField(0, v8::External::New(isolate, entity));
    info.GetReturnValue().Set(v8Entity);
}


} // namespace scripting
