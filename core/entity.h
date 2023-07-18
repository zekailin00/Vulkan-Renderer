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

    bool HasComponent(Component::Type type) const;
    Component* GetComponent(Component::Type type) const;

    void Serialize(Json::Value& json);
    bool Deserialize(Json::Value& json);

    void Update(Timestep ts);

    void ReparentTo(Entity* entity);

    bool operator==(const Entity& e);

public:
    const glm::mat4& GetGlobalTransform() const;
    const glm::mat4& GetLocalTransform() const;

    void SetLocalTransform(const glm::mat4& transform, bool isPhysicsDriven = false);
    /**
     * @brief Set the Local Transform
     * 
     * @param postion <x, y, z>
     * @param rotation <w, x, y, z> quaternion
     * @param scale <x, y, z>
     * @param isPhysicsDriven true if this is driven by physics system
     */
    void SetLocalTransform(const glm::vec3& postion, const glm::vec3& rotation,
        const glm::vec3& scale, bool isPhysicsDriven = false);

    glm::vec3 GetLocalTranslation() const;
    glm::vec3 GetLocalRotation() const;
    glm::vec3 GetLocalScale() const;

    //////////////// For Physics ///////////////

    glm::vec3 GetGlobalScale() const;
    glm::mat4 GetGlobalTransformNoScale() const;

    ////////////////////////////////////////////

    Entity* GetParent() const {return parent;}
    Entity* GetChildByName(std::string name) const;
    void ScanEntities(std::function<void(Entity*)> fn);
    const std::list<Entity*>& GetChildren();

    const std::string& GetName() const {return name;}
    void SetName(const std::string& name) {this->name = name;}
    Scene* GetScene() const {return scene;}

private:
    friend Scene;

    const Entity& operator=(const Entity&) = delete;
    Entity(const Entity&) = delete;

    void UpdateTransform(bool isPhysicsDriven);

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