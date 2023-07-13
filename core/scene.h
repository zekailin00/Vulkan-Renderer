#pragma once

#include "timestep.h"
#include "scene_contexts.h"
#include "entity.h"
#include "core_asset_manager.h"

#include <string>
#include <memory>


class Entity;

class Scene
{

public:
    enum class State
    {
        Inactive, // Not actively being rendered
        Editor, // active rendered in editor mode(No physics, scripts... running)
        Running,
        RunningVR,
        Paused
    };

public:
    static Scene* NewScene(std::string name, ICoreAssetManager* manager);
    static Scene* LoadFromFile(
        std::string path, ICoreAssetManager* manager, State state);
    bool SaveToFile(std::string path);
    Scene* Replicate(State state);

    std::shared_ptr<SceneContext> GetSceneContext(
        SceneContext::Type type);
    void SetSceneContext(
        SceneContext::Type type,
        std::shared_ptr<SceneContext> context);

    const std::string& GetSceneName() {return sceneName;}
    ICoreAssetManager* GetAssetManager() {return assetManager;}
    State GetState() {return state;}

    Entity* NewEntity();
    bool RemoveEntity(Entity* entity);
    Entity* GetEntityByName(std::string name);
    void GetEntitiesWithComponent(
        Component::Type type, std::vector<Entity*>& list);

    Entity* GetRootEntity();
    void Update(Timestep ts);

    ~Scene(); // Remove all entities

private:
    Scene() = default;
    Scene(const Scene&) = delete;
    const Scene& operator=(const Scene&) = delete;

    Entity* _NewEntity();

private:
    State state = State::Editor;
    std::string sceneName;
    Entity* rootEntity = nullptr;
    unsigned int entityCounter = 0;
    ICoreAssetManager* assetManager = nullptr;
    std::shared_ptr<SceneContext> contexts[SceneContext::Type::CtxSize] = {};
};