#include "environment/environment.h"

#include "environment/entity_callbacks.h"
#include "environment/scene_callbacks.h"
#include "environment/script_math.h"
#include "environment/input_callbacks.h"
#include "environment/component_callbacks.h"
#include "environment/asset_manager_callbacks.h"
#include "logger.h"

namespace scripting
{

v8::Local<v8::FunctionTemplate> MakeEntityTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::FunctionTemplate> temp = v8::FunctionTemplate::New(isolate);
    v8::Local<v8::ObjectTemplate> prototype = temp->PrototypeTemplate();

    temp->InstanceTemplate()->SetInternalFieldCount(1);

    prototype->Set(isolate, "AddComponent",
        v8::FunctionTemplate::New(isolate, AddComponent));
    prototype->Set(isolate, "RemoveComponent",
        v8::FunctionTemplate::New(isolate, RemoveComponent));
    prototype->Set(isolate, "HasComponent",
        v8::FunctionTemplate::New(isolate, HasComponent));
    prototype->Set(isolate, "GetComponent",
        v8::FunctionTemplate::New(isolate, GetComponent));
    prototype->Set(isolate, "ReparentTo",
        v8::FunctionTemplate::New(isolate, ReparentTo));
    prototype->Set(isolate, "GetGlobalTransform",
        v8::FunctionTemplate::New(isolate, GetGlobalTransform));
    prototype->Set(isolate, "GetLocalTransform",
        v8::FunctionTemplate::New(isolate, GetLocalTransform));
    prototype->Set(isolate, "SetLocalTransform1",
        v8::FunctionTemplate::New(isolate, SetLocalTransform1));
    prototype->Set(isolate, "SetLocalTransform2",
        v8::FunctionTemplate::New(isolate, SetLocalTransform2));
    prototype->Set(isolate, "GetLocalTranslation",
        v8::FunctionTemplate::New(isolate, GetLocalTranslation));
    prototype->Set(isolate, "GetLocalRotation",
        v8::FunctionTemplate::New(isolate, GetLocalRotation));
    prototype->Set(isolate, "GetLocalScale",
        v8::FunctionTemplate::New(isolate, GetLocalScale));
    prototype->Set(isolate, "GetParent",
        v8::FunctionTemplate::New(isolate, GetParent));
    prototype->Set(isolate, "GetChildByName",
        v8::FunctionTemplate::New(isolate, GetChildByName));
    prototype->Set(isolate, "GetChildren",
        v8::FunctionTemplate::New(isolate, GetChildren));
    prototype->Set(isolate, "GetName",
        v8::FunctionTemplate::New(isolate, GetName));
    prototype->Set(isolate, "SetName",
        v8::FunctionTemplate::New(isolate, SetName));
    prototype->Set(isolate, "GetScene",
        v8::FunctionTemplate::New(isolate, GetScene));

    return handleScope.Escape(temp);
}

v8::Local<v8::FunctionTemplate> MakeSceneTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::FunctionTemplate> temp = v8::FunctionTemplate::New(isolate);
    v8::Local<v8::ObjectTemplate> prototype = temp->PrototypeTemplate();

    temp->InstanceTemplate()->SetInternalFieldCount(1);

    prototype->Set(isolate, "GetSceneName",
        v8::FunctionTemplate::New(isolate, GetSceneName));
    prototype->Set(isolate, "NewEntity",
        v8::FunctionTemplate::New(isolate, NewEntity));
    prototype->Set(isolate, "RemoveEntity", //FIXME:
        v8::FunctionTemplate::New(isolate, RemoveEntity));
    prototype->Set(isolate, "GetEntityByName",
        v8::FunctionTemplate::New(isolate, GetEntityByName));
    prototype->Set(isolate, "GetEntitiesWithComponent",
        v8::FunctionTemplate::New(isolate, GetEntitiesWithComponent));
    prototype->Set(isolate, "GetRootEntity",
        v8::FunctionTemplate::New(isolate, GetRootEntity));

    return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeMathTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);

    temp->Set(isolate, "Add",
        v8::FunctionTemplate::New(isolate, math::Add));
    temp->Set(isolate, "Multiply",
        v8::FunctionTemplate::New(isolate, math::Multiply));
    temp->Set(isolate, "Subtract",
        v8::FunctionTemplate::New(isolate, math::Subtract));
    temp->Set(isolate, "Inverse",
        v8::FunctionTemplate::New(isolate, math::Inverse));
    temp->Set(isolate, "Cross",
        v8::FunctionTemplate::New(isolate, math::Cross));
    temp->Set(isolate, "Normalize",
        v8::FunctionTemplate::New(isolate, math::Normalize));
    temp->Set(isolate, "Mat4Identity",
        v8::FunctionTemplate::New(isolate, math::Mat4Identity));
    temp->Set(isolate, "Mat4Rotation",
        v8::FunctionTemplate::New(isolate, math::Mat4Rotation));
    temp->Set(isolate, "Mat4Translate",
        v8::FunctionTemplate::New(isolate, math::Mat4Translate));
    temp->Set(isolate, "Mat4Scale",
        v8::FunctionTemplate::New(isolate, math::Mat4Scale));

    return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeInputTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
    
    temp->SetInternalFieldCount(1);

    temp->Set(isolate, "LeftAimEvent",
        v8::FunctionTemplate::New(isolate, LeftAimEvent));
    temp->Set(isolate, "RightAimEvent",
        v8::FunctionTemplate::New(isolate, RightAimEvent));
    temp->Set(isolate, "LeftGripEvent",
        v8::FunctionTemplate::New(isolate, LeftGripEvent));
    temp->Set(isolate, "RightGripEvent",
        v8::FunctionTemplate::New(isolate, RightGripEvent));
    temp->Set(isolate, "ControllerEvent",
        v8::FunctionTemplate::New(isolate, ControllerEvent));

    return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeAssetManagerTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
    
    temp->SetInternalFieldCount(1);

    temp->Set(isolate, "GetMeshResourcePaths",
        v8::FunctionTemplate::New(isolate, GetMeshResourcePaths));

    return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeMeshCompTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
    
    temp->SetInternalFieldCount(1);

    temp->Set(isolate, "GetEntity",
        v8::FunctionTemplate::New(isolate, GetEntity));
    temp->Set(isolate, "GetComponentType",
        v8::FunctionTemplate::New(isolate, GetComponentType));
    temp->Set(isolate, "SetMeshResoucePath",
        v8::FunctionTemplate::New(isolate, SetMeshResoucePath));
    temp->Set(isolate, "GetMeshResourcePath",
        v8::FunctionTemplate::New(isolate, GetMeshResourcePath));

    return handleScope.Escape(temp);
}

v8::Local<v8::ObjectTemplate> MakeLightCompTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
    
    temp->SetInternalFieldCount(1);

    temp->Set(isolate, "GetEntity",
        v8::FunctionTemplate::New(isolate, GetEntity));
    temp->Set(isolate, "GetComponentType",
        v8::FunctionTemplate::New(isolate, GetComponentType));
    temp->Set(isolate, "SetLightColor",
        v8::FunctionTemplate::New(isolate, SetLightColor));

    return handleScope.Escape(temp);
}

void print(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    bool first = true;
    std::string output;
    for (int i = 0; i < args.Length(); i++)
    {
        v8::HandleScope handle_scope(args.GetIsolate());
        if (first)
        {
            first = false;
            output = "[Scripting] ";
        }
        else
        {
            output = output +  " ";
        }
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        output = output + *str;
    }
    
    Logger::Write(output,
        Logger::Level::Info, Logger::MsgType::Scripting);
}

} // namespace scripting