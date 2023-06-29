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
        Camera,
        // Drives local transform. Must be before components that use local transform.
        VrDisplay,
        Light,
        Mesh,
        UI,
        Wireframe,
        Script,
        Size
    };

    bool enabled = true;
    Type type = Type::Size;
    Entity* entity;

public:
    virtual void Update(Timestep ts) = 0;
    virtual void Serialize(Json::Value& json) = 0;
    virtual ~Component() = default;

    Component() = default;
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