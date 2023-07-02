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

    const v8::Global<v8::ObjectTemplate>& GetGlobalTemplate()
    {
        return globalTemplate;
    }

    const v8::Global<v8::ObjectTemplate>& GetSystemTemplate()
    {
        return systemTemplate;
    }

    const v8::Global<v8::FunctionTemplate>& GetEntityTemplate()
    {
        return entityTemplate;
    }

    const v8::Global<v8::FunctionTemplate>& GetComponentTemplate()
    {
        return componentTemplate;
    }

    const v8::Global<v8::FunctionTemplate>& GetSceneTemplate()
    {
        return sceneTemplate;
    }

    const v8::Global<v8::ObjectTemplate>& GetMeshCompTemplate()
    {
        return meshCompTemplate;
    }

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

    v8::Global<v8::ObjectTemplate> globalTemplate;
    v8::Global<v8::ObjectTemplate> systemTemplate;
    v8::Global<v8::ObjectTemplate> mathTemplate;
    v8::Global<v8::ObjectTemplate> inputTemplate;
    v8::Global<v8::ObjectTemplate> assetManagerTemplate;

    v8::Global<v8::ObjectTemplate> meshCompTemplate;

    v8::Global<v8::FunctionTemplate> entityTemplate;
    v8::Global<v8::FunctionTemplate> componentTemplate;
    v8::Global<v8::FunctionTemplate> sceneTemplate;
};

} // namespace scripting