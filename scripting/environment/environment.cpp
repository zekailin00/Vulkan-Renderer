#include "environment/environment.h"

#include "environment/entity_callbacks.h"
#include "logger.h"

namespace scripting
{

static void print(const v8::FunctionCallbackInfo<v8::Value>& args);


v8::Local<v8::ObjectTemplate> MakeSystemTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
    temp->Set(isolate, "print", v8::FunctionTemplate::New(isolate, print));

    return handleScope.Escape(temp);
}

v8::Local<v8::FunctionTemplate> MakeEntityTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::FunctionTemplate> temp = v8::FunctionTemplate::New(isolate);
    v8::Local<v8::ObjectTemplate> prototype = temp->PrototypeTemplate();

    prototype->SetInternalFieldCount(1);

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
            output = "[scripting] ";
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