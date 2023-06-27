#pragma once 

#include <libplatform/libplatform.h>
#include <v8-isolate.h>
#include <v8-context.h>
#include <v8-exception.h>
#include <v8-isolate.h>
#include <v8-local-handle.h>

#include <string>

#include "script_context.h"


namespace scripting
{

class ScriptingSubsystem
{
public:
    static std::string scriptResource;

public:
    static ScriptingSubsystem& GetInstance()
    {
        static ScriptingSubsystem scriptingSubsystem;
        return scriptingSubsystem;
    } 

    ScriptingSubsystem(ScriptingSubsystem const&) = delete;
    void operator=(ScriptingSubsystem const&) = delete;

    void Initialize(char* argv[]);

    ScriptContext* NewContext();

    /**
     * @brief It compiles Javascript code from the specified file
     * and extracts the instance of the function defined in the script.
     * @param path Path to the Javascript file.
     * @return The object instantiated by the function, or empty if failed.
     */
    v8::MaybeLocal<v8::Object> GetScriptObject(std::string path);


    void SetEntityContext(void* field);

    bool ExecuteScriptCallback(v8::Local<v8::Object> instance,
                               std::string callbackName,
                               float* timestep = nullptr);
    
    v8::Isolate* GetIsolate() {return isolate;}

    //https://stackoverflow.com/questions/33168903/c-scope-and-google-v8-script-context
private:
    bool initialized = false;
    v8::Isolate* isolate;
    v8::Isolate::CreateParams createParams;
    std::unique_ptr<v8::Platform> platform;
    v8::Global<v8::Context> context;

    v8::Global<v8::ObjectTemplate> globalTemplate;
    v8::Global<v8::ObjectTemplate> systemTemplate;
    v8::Global<v8::ObjectTemplate> entityTemplate;
    v8::Global<v8::ObjectTemplate> componentTemplate;
    v8::Global<v8::Object> entityObject;

private:
    ScriptingSubsystem() = default;
    ~ScriptingSubsystem();

    void ExceptionHandler(v8::Isolate* isolate, v8::TryCatch* try_catch);
    v8::Local<v8::Context> CreateEnvironment(v8::Isolate* isolate);
};

} // namespace scripting