#pragma once 

#include <libplatform/libplatform.h>
#include <v8-isolate.h>
#include <v8-context.h>
#include <v8-exception.h>
#include <v8-isolate.h>
#include <v8-local-handle.h>
#include <v8-persistent-handle.h>

#include <string>

#include "script_context.h"
#include "environment_templates.h"
#include "script_asset_manager.h"


namespace scripting
{

class ScriptingSystem
{
public:
    static std::string scriptResource;

public:
    // TODO: acquire asset manager from top level
    static ScriptingSystem* GetInstance()
    {
        static ScriptingSystem scriptingSystem;
        return &scriptingSystem;
    }

    void Initialize(char* argv[] = nullptr);

    ScriptContext* NewContext();

    v8::Isolate* GetIsolate() {return isolate;}

    ScriptingSystem(ScriptingSystem const&) = delete;
    void operator=(ScriptingSystem const&) = delete;

private:
    ScriptingSystem() = default;
    ~ScriptingSystem();

    void BuildEnvironment();

private:
    bool initialized = false;
    v8::Isolate* isolate;
    v8::Isolate::CreateParams createParams;
    std::unique_ptr<v8::Platform> platform;
};

} // namespace scripting