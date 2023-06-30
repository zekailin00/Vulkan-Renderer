#include "script_context.h"

#include <v8-script.h>
#include <v8-local-handle.h>
#include <v8-function.h>
#include <v8-value.h>

#include <memory>
#include <string>
#include <fstream>
#include <filesystem>

#include "filesystem.h"
#include "script_exception.h"
#include "validation.h"
#include "logger.h"


namespace scripting
{

Script* ScriptContext::NewScript(IScriptAssetManager* assetManager)
{
    Script* script = new Script();
    script->assetManager = assetManager;
    script->isolate = isolate;
    script->scriptContext = this;

    return script;
}

void ScriptContext::ExecuteFromPath(std::string path)
{
    std::string source;

    // std::filesystem::path fsPath = path;

    // if (!Filesystem::IsRegularFile(path))
    // {
    //     Logger::Write(
    //         "[Scripting] Invalid path.",
    //         Logger::Level::Warning, Logger::MsgType::Scripting);

    //     return;
    // }
    
    std::string buffer;
    std::ifstream file;
    file.open(path);
    if (file.good())
    {
        while(std::getline(file, buffer))
            source = source + buffer + "\n";
        file.close();

        this->Execute(source);
    }
    else
    {
        Logger::Write(
            "[Scripting] Invalid path.",
            Logger::Level::Warning, Logger::MsgType::Scripting);

        return;
    }
    
}

void ScriptContext::Execute(std::string source)
{
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
        isolate, GetContext());
    v8::Context::Scope contextScope(localContext);

    v8::Local<v8::String> v8Script =
        v8::String::NewFromUtf8(isolate, source.c_str())
        .ToLocalChecked();

    // Catch exceptions from compiling the script
    v8::TryCatch tryCatch(isolate);

    v8::ScriptOrigin origin(isolate, 
        v8::String::NewFromUtf8(isolate, "Terminal")
        .ToLocalChecked());

    v8::Local<v8::Script> script;
    v8::Local<v8::Value> result;

    if (!v8::Script::Compile(localContext, v8Script, &origin)
        .ToLocal(&script))
    {
        ASSERT(tryCatch.HasCaught());
        ExceptionHandler(&tryCatch, isolate);

        return;
    }
    else if (!script->Run(localContext).ToLocal(&result))
    {
        ASSERT(tryCatch.HasCaught());
        ExceptionHandler(&tryCatch, isolate);

        return;
    }

    v8::String::Utf8Value v8Output(isolate, result);
    std::string output = *v8Output;

    Logger::Write(
        "[Scripting] Output: " + output,
        Logger::Level::Info, Logger::MsgType::Scripting
    );

    return;
}

} // namespace scripting
