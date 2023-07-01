#pragma once

#include <v8-context.h>
#include <v8-isolate.h>
#include <v8-exception.h>
#include <v8-persistent-handle.h>

#include "script.h"
#include "script_asset_manager.h"
#include "scene_context.h"
#include "component.h"

#include <filesystem>


namespace scripting
{

class ScriptingSystem;

class ScriptContext: public SceneContext
{
public:
    Script* NewScript(IScriptAssetManager* assetManager);

    void Execute(std::string source);
    void ExecuteFromPath(std::string path);
    
    const v8::Persistent<v8::Context>& GetContext() {return context;}
    const v8::Persistent<v8::Object>& GetSystemObject()
    {
        return systemObject;
    }


    Component::Type GetSceneContextType() override
    {
        return Component::Type::Script;
    }

    ScriptContext(const ScriptContext&) = delete;
    void operator=(const ScriptContext&) = delete;

    ~ScriptContext()
    {
        context.Reset();
    }

private:
    ScriptContext() = default;
    friend ScriptingSystem;

private:
    v8::Persistent<v8::Context> context;
    v8::Persistent<v8::Object> systemObject;

    v8::Isolate* isolate = nullptr; // Owned by scripting system
};

} // namespace scripting