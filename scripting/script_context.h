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

class ScriptContext: SceneContext
{
public:
    Script* NewScript(std::string resourcePath,
        IScriptAssetManager* assetManager);
    Script* NewScript(IScriptAssetManager* assetManager);

    void Execute(std::string source);
    void ExecuteFromPath(std::string fullPath);
    
    const v8::Persistent<v8::Context>& GetContext() {return context;}

    Component::Type GetSceneContextType() override
    {
        return Component::Type::Script;
    }

    ScriptContext(const ScriptContext&) = delete;
    void operator=(const ScriptContext&) = delete;

private:
    ScriptContext() = default;
    ~ScriptContext()
    {
        context.Reset();
    }
    
    friend ScriptingSystem;

private:
    v8::Persistent<v8::Context> context;

    v8::Isolate* isolate = nullptr; // Owned by scripting system
};

} // namespace scripting