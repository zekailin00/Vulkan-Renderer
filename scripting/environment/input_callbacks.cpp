#include "input_callbacks.h"

#include <v8-function.h>
#include <v8-external.h>
#include <v8-primitive.h>
#include <v8-isolate.h>
#include <v8-value.h>
#include <v8-container.h>

#include "logger.h"
#include "validation.h"

namespace scripting
{

void LeftAimTransform(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    if (info.Length() != 1 || !info[0]->IsFunction())
    {
        Logger::Write(
            "[Scripting] LeftAimTransform parameters are invalid",
            Logger::Level::Warning, Logger::Scripting
        );

        return;
    }

    

}

void RightAimTransform(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void LeftGripTransform(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

void RightGripTransform(const v8::FunctionCallbackInfo<v8::Value> &info)
{

}

} // namespace scripting
