#pragma once

#include "timestep.h"
#include "component.h"

#include <json/json.h>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <list>
#include <functional>

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
    const glm::mat4& GetGlobalTransform();
    const glm::mat4& GetLocalTransform();

    void SetLocalTransform(const glm::mat4& transform);
    /**
     * @brief Set the Local Transform
     * 
     * @param postion <x, y, z>
     * @param rotation <w, x, y, z> quaternion
     * @param scale <x, y, z>
     */
    void SetLocalTransform(
        const glm::vec3& postion, const glm::quat& rotation, const glm::vec3& scale);

    glm::vec3 GetLocalTranslation();
    glm::quat GetLocalRotation();
    glm::vec3 GetLocalScale();

    Entity* GetChildByName(std::string name);
    void ScanEntities(std::function<void(Entity*)> fn);
    const std::list<Entity*>& GetChildren();

    const std::string& GetName() {return name;}
    void SetName(const std::string& name) {this->name = name;}

private:
    friend Scene;

    const Entity& operator=(const Entity&) = delete;
    Entity(const Entity&) = delete;

    Entity() = default;
    ~Entity() = default;

private:
    Component* componentList[(int)Component::Type::Size];
    Scene* scene = nullptr;
    std::string name;

    glm::mat4 localTransform;
    glm::mat4 globalTransform;

    Entity* parent;
    std::list<Entity*> children;
};