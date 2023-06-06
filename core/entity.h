#pragma once

#include "timestep.h"
#include "component.h"

#include <json/json.h>
#include <glm/mat4x4.hpp>
#include <list>

class Scene;

class Entity 
{
public:
    Component* AddComponent(Component::Type type);
    void RemoveComponent(Component::Type type);

    bool HasComponent(Component::Type type);
    Component* GetComponent(Component::Type type);

    void Serialize(Json::Value& json);
    bool Deserialize(Json::Value& json);

    void Update(Timestep ts);

    void ReparentTo(Entity* entity);

    bool operator==(const Entity& e);

public:
    std::string name;

private:
    friend Scene;

    const Entity& operator=(const Entity&) = delete;
    Entity(const Entity&) = delete;

    Entity() = default;
    ~Entity() = default;

    void SerializeMat4(Json::Value& json);
    void DeserializeMat4(Json::Value& json);

private:
    Component* componentList[(int)Component::Type::Size];
    Scene* scene = nullptr;

    glm::mat4 localTransform;
    glm::mat4 globalTransform;

    Entity* parent;
    std::list<Entity*> children;
};