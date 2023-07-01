#pragma once

#include <string>

#include <v8-local-handle.h>
#include <v8-value.h>
#include <v8-isolate.h>
#include <v8-context.h>
#include <v8-exception.h>
#include <v8-persistent-handle.h>

#include "event_queue.h"
#include "events.h"

#include "entity.h"
#include "timestep.h"
#include "script_asset_manager.h"
#include <list>

namespace scripting
{

class ScriptContext;
class Script
{

public:
    /**
     * @brief Load the source code at the resource path
     * in the workspace directory. If file is not found,
     * source, resourcePath, and scriptInstance are reset.
     * If compilation failed, source and resourcePath point
     * to the given values, but scriptInstance is reset.
     * 
     * @param resourcePath path to the source code 
     * relative to the workspace directory.
     * @return True if source code is found at the 
     * specified path. Compilation failure still returns true.
     */
    bool LoadSource(std::string resourcePath, Entity* entity);

    bool LoadSource(std::string resourcePath, std::string source, Entity* entity);
    std::string GetResourcePath() {return resourcePath;}

    bool HasScript() {return !(resourcePath == "None");}

    int AddEventSubscriber(std::function<void (Event *)> callback);

    /**
     * @brief Run callback "OnUpdated",
     * or "OnCreated" if called the first time
     * 
     * @param ts Timestep in float.
     */
    void Run(Timestep ts = 0.0f);

    void RunOnDestroyed();

    ~Script();
    Script(const Script&) = delete;
    void operator=(const Script&) = delete;

private:
    Script() = default;

    bool Compile(Entity* entity);
    void RunCallback(std::string callbackName, Timestep ts);
    void UnSubscribeAll();

    friend ScriptContext;

private:
    std::string resourcePath = "None";
    std::string source = "";
    v8::Persistent<v8::Object> scriptInstance;
    std::list<int> subscriberHandles;
    bool OnCreatedCalled = false;

    ScriptContext* scriptContext = nullptr;
    IScriptAssetManager* assetManager = nullptr;
    v8::Isolate* isolate = nullptr; // Owned by ScriptingSystem
};

} // namespace scripting