#pragma once

#include <PxPhysicsAPI.h>

#include "collision_shape.h"

namespace physics
{

class StaticRigidbody;
class DynamicRigidbody;

class PhysicsContext
{

public:
    PhysicsContext(physx::PxPhysics* gPhysics);
    ~PhysicsContext();

    StaticRigidbody* NewStaticRigidbody();
    DynamicRigidbody* NewDynamicRigidbody();

    CollisionShape* AddCollisionShape(
        DynamicRigidbody* ridigbody,
        GeometryType geometryType
    );

private:
    physx::PxDefaultCpuDispatcher* gDispatcher;
    physx::PxScene* gScene;

    //Owned by physics system.
    physx::PxPhysics* gPhysics;
};

} // namespace physics