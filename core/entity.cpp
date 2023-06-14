#include "entity.h"

#include "scene.h"
#include "validation.h"
#include "serialization.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>


Component* Entity::AddComponent(Component::Type type)
{
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

bool Entity::HasComponent(Component::Type type)
{
    return componentList[(int)type] != nullptr;
}

Component* Entity::GetComponent(Component::Type type)
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
}

const glm::mat4& Entity::GetGlobalTransform()
{
    static const glm::mat4 IDENTITY(1.0f);
    if (parent == nullptr)
    {
        ASSERT(this == scene->GetRootEntity());
        return IDENTITY;
    }

    globalTransform = parent->GetGlobalTransform() * localTransform;
    return globalTransform;
}

const glm::mat4& Entity::GetLocalTransform()
{
    return localTransform;
}

void Entity::SetLocalTransform(const glm::mat4& transform)
{
    localTransform = transform;
}

void Entity::SetLocalTransform(const glm::vec3& postion,
    const glm::quat& rotation, const glm::vec3& scale)
{
    // Transform = T * R * S
    localTransform = glm::translate(glm::mat4(1.0f), postion) *
                     glm::toMat4(rotation) *
                     glm::scale(glm::mat4(1.0f), scale);
}

glm::vec3 Entity::GetLocalTranslation()
{
    return glm::vec3(localTransform[3]);
}

glm::quat Entity::GetLocalRotation()
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(
        localTransform, scale, rotation,
        translation, skew, perspective
    );

    return rotation;
}

glm::vec3 Entity::GetLocalScale()
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(
        localTransform, scale, rotation,
        translation, skew, perspective
    );

    return scale;
}

Entity* Entity::GetChildByName(std::string name)
{
    Entity* childResult;
    for (Entity* e: children)
    {
        if (e->name == name)
            return e;
        if (childResult = e->GetChildByName(name))
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