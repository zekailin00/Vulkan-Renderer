#pragma once

#include "timestep.h"
#include "component.h"

#include <json/json.h>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
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
    glm::mat4 GetGlobalTransform();
    glm::mat4 GetLocalTransform();

    void SetLocalTransform(glm::mat4 transform);
    void SetLocalTransform(glm::vec3 postion, glm::quat rotation, glm::vec3 scale);

    glm::vec3 GetLocalTranslation();
    glm::quat GetLocalRotation();
    glm::vec3 GetLocalScale();

    Entity* GetChildByName(std::string name);
    std::list<Entity*>& GetChildren();

public:
    std::string name;

private:
    friend Scene;

    const Entity& operator=(const Entity&) = delete;
    Entity(const Entity&) = delete;

    Entity() = default;
    ~Entity() = default;

private:
    Component* componentList[(int)Component::Type::Size];
    Scene* scene = nullptr;

    glm::mat4 localTransform;
    glm::mat4 globalTransform;

    Entity* parent;
    std::list<Entity*> children;
};