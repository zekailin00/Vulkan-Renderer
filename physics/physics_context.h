#pragma once

#include <PxPhysicsAPI.h>

#include "collision_shape.h"
#include "timestep.h"
#include "scene_contexts.h"


namespace physics
{

class StaticRigidbody;
class DynamicRigidbody;

class PhysicsContext: ScenePhysicsContext
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

    int Simulate(Timestep ts) override;

private:
    physx::PxDefaultCpuDispatcher* gDispatcher;
    physx::PxScene* gScene;

    float accumulator = 0.0f;
    const float STEP_SIZE = 1.0f / 60.0f;

    //Owned by physics system.
    physx::PxPhysics* gPhysics;
};

} // namespace physics