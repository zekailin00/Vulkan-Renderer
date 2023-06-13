#pragma once

#include "timestep.h"
#include "entity.h"

#include <string>


class Entity;

class Scene
{

public:
    enum class State
    {
        Inactive, // Not actively being rendered
        Editor, // active rendered in editor mode(No physics, scripts... running)
        Running,
        Paused
    };

public:
    static Scene* NewScene(std::string name);
    static Scene* LoadFromFile(std::string path, State state);
    bool SaveToFile(std::string path);
    Scene* Replicate(State state);

    const std::string& GetSceneName() {return sceneName;}

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
};