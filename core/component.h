#pragma once

#include "timestep.h"
#include <json/json.h>
#include <functional>


class Entity;
struct Component;

typedef std::function<Component*(Entity*)> CompInitializer;
typedef std::function<Component*(Entity*, Json::Value&)> CompDeserializer;

struct Component
{
    enum class Type
    {
        CompA,
        CompB,
        Size
    };

    bool enabled = true;
    Type type = Type::Size;

    Entity* entity;

    virtual void OnCreated() = 0;
    virtual void OnUpdated(Timestep ts) = 0;
    virtual void OnDestroyed() = 0;
    virtual void Serialize(Json::Value& json) = 0;
    virtual bool Deserialize(Json::Value& json) = 0;
    virtual Component* replicate() = 0;

    Component() = default;
    virtual ~Component() = default;
};

class ComponentLocator
{

public:
    static void SetInitializer(Component::Type type, CompInitializer callback)
    {
        initializerList[(int)type] = callback;
    }

    static CompInitializer GetInitializer(Component::Type type)
    {
        return initializerList[(int)type];
    }

    static void SetDeserializer(Component::Type type, CompDeserializer callback)
    {
        deserializerList[(int)type] = callback;
    }

    static CompDeserializer GetDeserializer(Component::Type type)
    {
        return deserializerList[(int)type];
    }

private:
    static CompInitializer initializerList[(int)Component::Type::Size];
    static CompDeserializer deserializerList[(int)Component::Type::Size];
};