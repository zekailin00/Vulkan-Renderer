#pragma once

#include <v8-context.h>
#include <v8-isolate.h>
#include <v8-exception.h>
#include <v8-persistent-handle.h>

#include "script.h"
#include "script_asset_manager.h"

namespace scripting
{

class ScriptingSystem;

class ScriptContext
{
public:
    Script* NewScript(std::string resourcePath);
    Script* NewScript();
    
    const v8::Persistent<v8::Context>& GetContext() {return context;}

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

    IScriptAssetManager* assetManager = nullptr;
    v8::Isolate* isolate = nullptr; // Owned by scripting system
};

} // namespace scripting