#pragma once

#include <v8-exception.h>
#include <v8-local-handle.h>
#include <v8-primitive.h>

#include "logger.h"

#include <string>

namespace scripting
{

static void ExceptionHandler(v8::TryCatch* tryCatch, v8::Isolate* isolate)
{
    v8::HandleScope handleScope(isolate);
    v8::String::Utf8Value exception(isolate, tryCatch->Exception());

    const char* exceptionString = *exception;
    v8::Local<v8::Message> message = tryCatch->Message();

    if (message.IsEmpty())
    {
        Logger::Write(exceptionString,
            Logger::Level::Warning, Logger::MsgType::Scripting);
    }
    else 
    {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(isolate,
            message->GetScriptOrigin().ResourceName());
        v8::Local<v8::Context> context(isolate->GetCurrentContext());
        const char* filenameString = *filename;
        int linenum = message->GetLineNumber(context).FromJust();
        std::string msg = filenameString;
        msg = msg + ":";
        msg = msg + std::to_string(linenum)+": "+exceptionString;
        Logger::Write(msg,
            Logger::Level::Warning, Logger::MsgType::Scripting);

        // Print line of source code.
        v8::String::Utf8Value sourceline(isolate, 
            message->GetSourceLine(context).ToLocalChecked());
        const char* sourcelineString = *sourceline;
        Logger::Write(sourcelineString,
            Logger::Level::Warning, Logger::MsgType::Scripting);

        // Print wavy underline (GetUnderline is deprecated).
        std::string underline = "";
        int start = message->GetStartColumn(context).FromJust();
        for (int i = 0; i < start; i++)
        {
            underline = underline + " ";
        }
        int end = message->GetEndColumn(context).FromJust();
        for (int i = start; i < end; i++)
        {
            underline = underline + "^";
        }
        underline = underline + "\n";
        Logger::Write(underline,
            Logger::Level::Warning, Logger::MsgType::Scripting);

        v8::Local<v8::Value> stackTraceString;
        if (tryCatch->StackTrace(context).ToLocal(&stackTraceString) &&
            stackTraceString->IsString() &&
            stackTraceString.As<v8::String>()->Length() > 0)
        {
            v8::String::Utf8Value stackTrace(isolate, stackTraceString);
            const char* err = *(stackTrace);
            Logger::Write(err,
                Logger::Level::Warning, Logger::MsgType::Scripting);
        }
    }
}

} // namespace scripting