#include "entity.h"

#include "scene.h"
#include "validation.h"
#include "serialization.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>


Component* Entity::AddComponent(Component::Type type)
{
    ASSERT(componentList[(int)type] == nullptr);
    Component* component = ComponentLocator::GetInitializer(type)(this);
    componentList[(int)type] = component;

    ASSERT(component->entity == this);
    ASSERT(component->type == type);

    return component;
}

void Entity::RemoveComponent(Component::Type type)
{
    if (!HasComponent(type))
        return;

    Component* component = componentList[(int)type];
    componentList[(int)type] = nullptr;
    delete component;
}

bool Entity::HasComponent(Component::Type type) const
{
    return componentList[(int)type] != nullptr;
}

Component* Entity::GetComponent(Component::Type type) const
{
    return componentList[(int)type];
}

bool Entity::operator==(const Entity& e)
{
    assert(this->scene== e.scene);
    return this == &e;
}

void Entity::Serialize(Json::Value& json)
{
    json["name"] = name;
    SerializeMat4(localTransform, json["localTransform"]);
    
    Json::Value& jsonComponents = json["components"];
    for(int i = 0; i < (int)Component::Type::Size; i++)
    {
        if (componentList[i])
        {
            Json::Value& jsonComponent = jsonComponents[i];
            componentList[i]->Serialize(jsonComponent);
        }
    }

    Json::Value& jsonChildren = json["children"];
    int index = 0;
    for(Entity* e: children)
    {
        int i = index++;
        Json::Value& jsonchild = jsonChildren[i];
        e->Serialize(jsonchild);
    }
}

bool Entity::Deserialize(Json::Value& json)
{
    name = json["name"].asString();
    DeserializeMat4(localTransform, json["localTransform"]);

    Json::Value jsonComponents = json["components"];
    for (int i = 0; i < (int)Component::Type::Size; i++)
    {
        if (!jsonComponents[i].isNull())
        {
            Json::Value& jsonComponent = jsonComponents[i];
            componentList[i] = ComponentLocator::GetDeserializer(
                (Component::Type)i)(this, jsonComponent);

            ASSERT(componentList[i]->entity == this);
            ASSERT((int)componentList[i]->type == i);
        }
    }

    UpdateLocalEulerXYZ();
    UpdateTransform(true);

    Json::Value& jsonChildren = json["children"];
    for (int i = 0; i < jsonChildren.size(); i++)
    {
        Json::Value& jsonChild = jsonChildren[i];

        Entity* entity = scene->NewEntity();
        entity->ReparentTo(this);
        entity->Deserialize(jsonChild);
    }

    return true;
}

void Entity::Update(Timestep ts)
{
    for (int i = 0; i < (int)Component::Type::Size; i++)
        if (componentList[i])
            componentList[i]->Update(ts);

    for (Entity* e: children)
        e->Update(ts);
}

void Entity::ReparentTo(Entity* entity)
{
    ASSERT(this->scene == entity->scene);

    parent->children.remove(this);
    entity->children.push_back(this);
    parent = entity;

    UpdateTransform(false);
}

const glm::mat4& Entity::GetGlobalTransform() const
{
    static const glm::mat4 IDENTITY(1.0f);
    if (parent == nullptr)
    {
        ASSERT(this == scene->GetRootEntity());
        return IDENTITY;
    }

    return globalTransform;
}

const glm::mat4& Entity::GetLocalTransform() const
{
    return localTransform;
}

void Entity::SetLocalTransform(const glm::mat4& transform, bool isPhysicsDriven)
{
    localTransform = transform;

    UpdateLocalEulerXYZ();

    UpdateTransform(isPhysicsDriven);
}

void Entity::SetLocalTransform(const glm::vec3& postion,
    const glm::vec3& eulerXYZ, const glm::vec3& scale, bool isPhysicsDriven)
{
    // Transform = T * R * S
    localTransform = glm::translate(glm::mat4(1.0f), postion) *
                     glm::eulerAngleXYZ(eulerXYZ[0], eulerXYZ[1], eulerXYZ[2]) *
                     glm::scale(glm::mat4(1.0f), scale);
    
    localEulerXYZ = eulerXYZ;

    UpdateTransform(isPhysicsDriven);
}

void Entity::SetLocalRotation(const glm::vec3& eulerXYZ)
{
    localEulerXYZ = eulerXYZ;

    glm::mat4 newTransform = glm::eulerAngleXYZ(
        eulerXYZ[0], eulerXYZ[1], eulerXYZ[2]
    );

    localTransform[0] = newTransform[0] * glm::length(localTransform[0]);
    localTransform[1] = newTransform[1] * glm::length(localTransform[1]);
    localTransform[2] = newTransform[2] * glm::length(localTransform[2]);

    UpdateTransform(false);
}

glm::vec3 Entity::GetLocalTranslation() const
{
    return glm::vec3(localTransform[3]);
}

glm::vec3 Entity::GetLocalRotation() const
{
    return localEulerXYZ;
}

glm::vec3 Entity::GetLocalScale() const
{
    return glm::vec3
    {
        glm::length(localTransform[0]),
        glm::length(localTransform[1]),
        glm::length(localTransform[2])
    };
}

glm::vec3 Entity::GetGlobalScale() const
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(
        globalTransform, scale, rotation,
        translation, skew, perspective
    );

    return scale;
}

glm::mat4 Entity::GetGlobalTransformNoScale() const
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(
        globalTransform, scale, rotation,
        translation, skew, perspective
    );

    glm::mat4 matRotation = glm::toMat4(rotation);
    glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f), translation);

    return matTranslate * matRotation;
}

Entity* Entity::GetChildByName(std::string name) const
{
    Entity* childResult;
    for (Entity* e: children)
    {
        if (e->name == name)
            return e;
        if ((childResult = e->GetChildByName(name)))
            return childResult;
    }

    return nullptr;
}

void Entity::ScanEntities(std::function<void(Entity*)> fn)
{
    fn(this);
    
    for (Entity* e: children)
    {
        e->ScanEntities(fn);
    }
}

const std::list<Entity*>& Entity::GetChildren()
{
    return children;
}

void Entity::UpdateTransform(bool isPhysicsDriven)
{
    static const glm::mat4 IDENTITY(1.0f);

    globalTransform =
        (parent?(parent->GetGlobalTransform()):IDENTITY) *
        localTransform;

    if (!isPhysicsDriven &&
        scene->GetSceneContext(SceneContext::Type::PhysicsCtx) != nullptr)
    {
        std::shared_ptr<ScenePhysicsContext> physicsCtx =
            std::dynamic_pointer_cast<ScenePhysicsContext>(
                scene->GetSceneContext(SceneContext::Type::PhysicsCtx)
            );

        physicsCtx->UpdatePhysicsTransform(this);
    }

    for (auto& e: children)
    {
        e->UpdateTransform(isPhysicsDriven);
    }
}

void Entity::UpdateLocalEulerXYZ()
{
    glm::mat4 transform
    {
        glm::normalize(localTransform[0]),
        glm::normalize(localTransform[1]),
        glm::normalize(localTransform[2]),
        localTransform[3]
    };

    glm::vec3 rot;
    glm::extractEulerAngleXYZ(transform, rot[0], rot[1], rot[2]);

    localEulerXYZ = rot;
}