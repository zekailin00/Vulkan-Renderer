#pragma once

#include <PxPhysicsAPI.h>

namespace physics
{

class PhysicsActor
{
public:
    enum ActorType
    {
        Static,
        Dynamic,
        Kinematic
    };

public:
    ActorType GetType();

private:
    physx::PxRigidActor* gRigidActor;
};

} // namespace physics
