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
        Editor,
        Running,
        Paused
    };

public:
    static Scene* NewScene();
    static Scene* LoadFromFile(std::string path, State state);
    bool SaveToFile(std::string path);
    Scene* Replicate(State state);

    Entity* NewEntity();
    bool RemoveEntity(Entity* entity);
    Entity* GetEntityByName(std::string name);

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
    Entity* rootEntity = nullptr;
    unsigned int entityCounter = 0;
};