#include "entity.h"

#include "scene.h"
#include "validation.h"


Component* Entity::AddComponent(Component::Type type)
{
    Component* component = ComponentLocator::GetInitializer(type)(this);
    componentList[(int)type] = component;

    ASSERT(component->entity == this);
    ASSERT(component->type == type);
    component->OnCreated();

    return component;
}

void Entity::RemoveComponent(Component::Type type)
{
    if (!HasComponent(type))
        return;

    Component* component = componentList[(int)type];
    componentList[(int)type] = nullptr;
    component->OnDestroyed();
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
    SerializeMat4(json["localTransform"]);
    
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
    DeserializeMat4(json["localTransform"]);

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
            componentList[i]->OnCreated();
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
            componentList[i]->OnUpdated(ts);

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

void Entity::SerializeMat4(Json::Value& json)
{
    json["00"] = localTransform[0][0];
    json["01"] = localTransform[0][1];
    json["02"] = localTransform[0][2];
    json["03"] = localTransform[0][3];
    json["10"] = localTransform[1][0];
    json["11"] = localTransform[1][1];
    json["12"] = localTransform[1][2];
    json["13"] = localTransform[1][3];
    json["20"] = localTransform[2][0];
    json["21"] = localTransform[2][1];
    json["22"] = localTransform[2][2];
    json["23"] = localTransform[2][3];
    json["30"] = localTransform[3][0];
    json["31"] = localTransform[3][1];
    json["32"] = localTransform[3][2];
    json["33"] = localTransform[3][3];
}

void Entity::DeserializeMat4(Json::Value& json)
{
    localTransform[0][0] = json["00"].asFloat();
    localTransform[0][1] = json["01"].asFloat();
    localTransform[0][2] = json["02"].asFloat();
    localTransform[0][3] = json["03"].asFloat();

    localTransform[1][0] = json["10"].asFloat();
    localTransform[1][1] = json["11"].asFloat();
    localTransform[1][2] = json["12"].asFloat();
    localTransform[1][3] = json["13"].asFloat();

    localTransform[2][0] = json["20"].asFloat();
    localTransform[2][1] = json["21"].asFloat();
    localTransform[2][2] = json["22"].asFloat();
    localTransform[2][3] = json["23"].asFloat();

    localTransform[3][0] = json["30"].asFloat();
    localTransform[3][1] = json["31"].asFloat();
    localTransform[3][2] = json["32"].asFloat();
    localTransform[3][3] = json["33"].asFloat();
}