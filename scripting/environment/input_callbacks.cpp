#include "input_callbacks.h"

#include <v8-function.h>
#include <v8-external.h>
#include <v8-primitive.h>
#include <v8-isolate.h>
#include <v8-value.h>
#include <v8-container.h>

#include "logger.h"
#include "validation.h"

#include "script.h"
#include "script_exception.h"
#include "script_math.h"


namespace scripting
{

void LeftAimEvent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() != 1 || !info[0]->IsFunction())
    {
        Logger::Write(
            "[Scripting] LeftAimEvent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> holder = info.Holder();
    v8::Local<v8::External> field =
        holder->GetInternalField(0).As<v8::External>();
    Script* script = static_cast<Script*>(field->Value());
    ASSERT(script != nullptr);
    
    int handle = script->AddEventSubscriber(
        [&info] (Event* event)
        {
            v8::Isolate* isolate = info.GetIsolate();
            v8::HandleScope handleScope(isolate);
            v8::Local<v8::Context> context = isolate->GetCurrentContext();

            if (event->type == Event::Type::LeftAimPose)
            {
                EventLeftAimPose* e =
                    dynamic_cast<EventLeftAimPose*>(event);
                
                v8::Local<v8::Value> v8Transfrom;
                v8Transfrom = toV8(e->transform, info).As<v8::Value>();

                v8::Local<v8::Function> callback = info[0].As<v8::Function>();

                v8::TryCatch tryCatch(isolate);
                v8::Local<v8::Value> result;

                if (!callback->Call(context, context->Global(), 1,&v8Transfrom)
                    .ToLocal(&result))
                {
                    ASSERT(tryCatch.HasCaught());
                    ExceptionHandler(&tryCatch, isolate);
                }
            }
        }
    );

    info.GetReturnValue().Set(handle);
}

void RightAimEvent(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void LeftGripEvent(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void RightGripEvent(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

} // namespace scripting
