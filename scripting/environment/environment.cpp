#include "environment/environment.h"

#include "logger.h"

  
static void print(const v8::FunctionCallbackInfo<v8::Value>& args);


v8::Local<v8::ObjectTemplate> MakeSystemTemplate(v8::Isolate* isolate)
{
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::ObjectTemplate> temp = v8::ObjectTemplate::New(isolate);
    temp->Set(isolate, "print", 
    v8::FunctionTemplate::New(isolate, print));

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
            output = "[Javascript] ";
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
