#include "input_callbacks.h"

#include <v8-function.h>
#include <v8-external.h>
#include <v8-primitive.h>
#include <v8-isolate.h>
#include <v8-value.h>
#include <v8-container.h>

#include "input_keycode.h"
#include "logger.h"
#include "validation.h"

#include "script.h"
#include "script_context.h"
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

    Script::SubscriberContext* subscriberContext = new Script::SubscriberContext();
    subscriberContext->function.Reset(isolate, info[0].As<v8::Function>());
    
    int handle = script->AddEventSubscriber(subscriberContext,
        [subscriberContext] (Event* event)
        {
            v8::HandleScope handleScope(subscriberContext->isolate);
            v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
                subscriberContext->isolate,
                subscriberContext->scriptContext->GetContext()
            );
            v8::Context::Scope contextScope(localContext);

            if (event->type == Event::Type::LeftAimPose)
            {
                EventLeftAimPose* e =
                    dynamic_cast<EventLeftAimPose*>(event);
                
                v8::Local<v8::Value> v8Transfrom;
                v8Transfrom = toV8(
                    e->transform, subscriberContext->isolate).As<v8::Value>();

                v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(
                    subscriberContext->isolate,
                    subscriberContext->function
                );

                v8::TryCatch tryCatch(subscriberContext->isolate);
                v8::Local<v8::Value> result;

                if (!callback->Call(localContext, localContext->Global(), 1,&v8Transfrom)
                    .ToLocal(&result))
                {
                    ASSERT(tryCatch.HasCaught());
                    ExceptionHandler(&tryCatch, subscriberContext->isolate);
                }
            }
        }
    );

    info.GetReturnValue().Set(handle);
}

void RightAimEvent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
if (info.Length() != 1 || !info[0]->IsFunction())
    {
        Logger::Write(
            "[Scripting] RightAimEvent parameters are invalid",
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

    Script::SubscriberContext* subscriberContext = new Script::SubscriberContext();
    subscriberContext->function.Reset(isolate, info[0].As<v8::Function>());
    
    int handle = script->AddEventSubscriber(subscriberContext,
        [subscriberContext] (Event* event)
        {
            v8::HandleScope handleScope(subscriberContext->isolate);
            v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
                subscriberContext->isolate,
                subscriberContext->scriptContext->GetContext()
            );
            v8::Context::Scope contextScope(localContext);

            if (event->type == Event::Type::RightAimPose)
            {
                EventRightAimPose* e =
                    dynamic_cast<EventRightAimPose*>(event);
                
                v8::Local<v8::Value> v8Transfrom;
                v8Transfrom = toV8(
                    e->transform, subscriberContext->isolate).As<v8::Value>();

                v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(
                    subscriberContext->isolate,
                    subscriberContext->function
                );

                v8::TryCatch tryCatch(subscriberContext->isolate);
                v8::Local<v8::Value> result;

                if (!callback->Call(localContext, localContext->Global(), 1,&v8Transfrom)
                    .ToLocal(&result))
                {
                    ASSERT(tryCatch.HasCaught());
                    ExceptionHandler(&tryCatch, subscriberContext->isolate);
                }
            }
        }
    );

    info.GetReturnValue().Set(handle);
}

void LeftGripEvent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
if (info.Length() != 1 || !info[0]->IsFunction())
    {
        Logger::Write(
            "[Scripting] LeftGripEvent parameters are invalid",
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

    Script::SubscriberContext* subscriberContext = new Script::SubscriberContext();
    subscriberContext->function.Reset(isolate, info[0].As<v8::Function>());
    
    int handle = script->AddEventSubscriber(subscriberContext,
        [subscriberContext] (Event* event)
        {
            v8::HandleScope handleScope(subscriberContext->isolate);
            v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
                subscriberContext->isolate,
                subscriberContext->scriptContext->GetContext()
            );
            v8::Context::Scope contextScope(localContext);

            if (event->type == Event::Type::LeftGripPose)
            {
                EventLeftGripPose* e =
                    dynamic_cast<EventLeftGripPose*>(event);
                
                v8::Local<v8::Value> v8Transfrom;
                v8Transfrom = toV8(
                    e->transform, subscriberContext->isolate).As<v8::Value>();

                v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(
                    subscriberContext->isolate,
                    subscriberContext->function
                );

                v8::TryCatch tryCatch(subscriberContext->isolate);
                v8::Local<v8::Value> result;

                if (!callback->Call(localContext, localContext->Global(), 1,&v8Transfrom)
                    .ToLocal(&result))
                {
                    ASSERT(tryCatch.HasCaught());
                    ExceptionHandler(&tryCatch, subscriberContext->isolate);
                }
            }
        }
    );

    info.GetReturnValue().Set(handle);
}

void RightGripEvent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
if (info.Length() != 1 || !info[0]->IsFunction())
    {
        Logger::Write(
            "[Scripting] RightGripEvent parameters are invalid",
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

    Script::SubscriberContext* subscriberContext = new Script::SubscriberContext();
    subscriberContext->function.Reset(isolate, info[0].As<v8::Function>());
    
    int handle = script->AddEventSubscriber(subscriberContext,
        [subscriberContext] (Event* event)
        {
            v8::HandleScope handleScope(subscriberContext->isolate);
            v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
                subscriberContext->isolate,
                subscriberContext->scriptContext->GetContext()
            );
            v8::Context::Scope contextScope(localContext);

            if (event->type == Event::Type::RightGripPose)
            {
                EventRightGripPose* e =
                    dynamic_cast<EventRightGripPose*>(event);
                
                v8::Local<v8::Value> v8Transfrom;
                v8Transfrom = toV8(
                    e->transform, subscriberContext->isolate).As<v8::Value>();

                v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(
                    subscriberContext->isolate,
                    subscriberContext->function
                );

                v8::TryCatch tryCatch(subscriberContext->isolate);
                v8::Local<v8::Value> result;

                if (!callback->Call(localContext, localContext->Global(), 1,&v8Transfrom)
                    .ToLocal(&result))
                {
                    ASSERT(tryCatch.HasCaught());
                    ExceptionHandler(&tryCatch, subscriberContext->isolate);
                }
            }
        }
    );

    info.GetReturnValue().Set(handle);
}


void ControllerEvent(const v8::FunctionCallbackInfo<v8::Value> &info)
{
if (info.Length() != 2 || !info[0]->IsInt32() || !info[1]->IsFunction())
    {
        Logger::Write(
            "[Scripting] ControllerEvent parameters are invalid",
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

    int keyCodeValue = info[0].As<v8::Int32>()->Value();
    KeyCode keyCode = (KeyCode)keyCodeValue;
    if (keyCode < KeyCode::LEFT_SQUEEZE || keyCode > KeyCode::RIGHT_B_CLICK)
    {
         Logger::Write(
            "[Scripting] ControllerEvent parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    Script::SubscriberContext* subscriberContext = new Script::SubscriberContext();
    subscriberContext->function.Reset(isolate, info[1].As<v8::Function>());
    
    int handle = script->AddEventSubscriber(subscriberContext,
        [subscriberContext, keyCode] (Event* event)
        {
            v8::HandleScope handleScope(subscriberContext->isolate);
            v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
                subscriberContext->isolate,
                subscriberContext->scriptContext->GetContext()
            );
            v8::Context::Scope contextScope(localContext);

            if (event->type == Event::Type::ControllerInput)
            {
                EventControllerInput* e =
                    dynamic_cast<EventControllerInput*>(event);
                if (e->keycode != keyCode)
                {
                    return;
                }

                v8::Local<v8::Value> arg;

                if ((keyCode >= KeyCode::LEFT_SQUEEZE && 
                     keyCode <= KeyCode::LEFT_THUMBSTICK_Y) ||
                    (keyCode >= KeyCode::RIGHT_SQUEEZE && 
                     keyCode <= KeyCode::RIGHT_THUMBSTICK_Y))
                {
                    arg = v8::Number::New(subscriberContext->isolate, e->floatValue);
                }
                else
                {
                    ASSERT((keyCode >= KeyCode::LEFT_X_CLICK && 
                            keyCode <= KeyCode::LEFT_MENU_CLICK) ||
                           (keyCode >= KeyCode::RIGHT_THUMBSTICK_CLICK && 
                            keyCode <= KeyCode::RIGHT_B_CLICK));
                    arg = v8::Boolean::New(subscriberContext->isolate, e->boolValue);
                }

                v8::Local<v8::Function> callback = v8::Local<v8::Function>::New(
                    subscriberContext->isolate,
                    subscriberContext->function
                );

                v8::TryCatch tryCatch(subscriberContext->isolate);
                v8::Local<v8::Value> result;

                if (!callback->Call(localContext, localContext->Global(), 1, &arg)
                    .ToLocal(&result))
                {
                    ASSERT(tryCatch.HasCaught());
                    ExceptionHandler(&tryCatch, subscriberContext->isolate);
                }
            }
        }
    );

    info.GetReturnValue().Set(handle);
}

} // namespace scripting
