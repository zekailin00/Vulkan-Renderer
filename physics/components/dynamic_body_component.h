#pragma once

#include "component.h"
#include "dynamic_rigidbody.h"

namespace physics
{

class PhysicsSystem;
class DynamicRigidbody;

class DynamicBodyInitializer
{
    PhysicsSystem* system;

public:
    DynamicBodyInitializer(PhysicsSystem* system):
        system(system) {}

    Component* operator()(Entity* entity);
};

class DynamicBodyDeserializer
{
    PhysicsSystem* system;

public:
    DynamicBodyDeserializer(PhysicsSystem* system):
        system(system) {}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct DynamicBodyComponent: public Component
{
    DynamicRigidbody* dynamicBody;
    

    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~DynamicBodyComponent() override;
};

}