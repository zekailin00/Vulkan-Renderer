#pragma once

#include "component.h"

#include "static_rigidbody.h"

namespace physics
{

class PhysicsSystem;
class StaticRigidbody;

class StaticBodyInitializer
{
    PhysicsSystem* system;

public:
    StaticBodyInitializer(PhysicsSystem* system):
        system(system) {}

    Component* operator()(Entity* entity);
};

class StaticBodyDeserializer
{
    PhysicsSystem* system;

public:
    StaticBodyDeserializer(PhysicsSystem* system):
        system(system) {}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct StaticBodyComponent: public Component
{
    StaticRigidbody* staticBody;
    

    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~StaticBodyComponent() override;
};

}