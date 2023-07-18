#pragma once

#include "component.h"

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

public:
    DynamicBodyDeserializer() {}

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