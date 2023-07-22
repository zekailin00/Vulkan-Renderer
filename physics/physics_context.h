#pragma once

#include <PxPhysicsAPI.h>

#include "collision_shape.h"
#include "timestep.h"
#include "scene_contexts.h"


namespace physics
{

class StaticRigidbody;
class DynamicRigidbody;
class Rigidbody;

class PhysicsContext: public ScenePhysicsContext
{

public:
    PhysicsContext(physx::PxPhysics* gPhysics);
    ~PhysicsContext();

    StaticRigidbody* NewStaticRigidbody();
    DynamicRigidbody* NewDynamicRigidbody();

    int Simulate(Timestep ts) override;

    void UpdatePhysicsTransform(Entity* e) override;

private:
    void RemoveRigidbody(physx::PxRigidActor* actor);
    CollisionShape* AddCollisionShape(GeometryType geometryType);

    friend StaticRigidbody;
    friend DynamicRigidbody;
    friend Rigidbody;

private:
    physx::PxDefaultCpuDispatcher* gDispatcher;
    physx::PxScene* gScene;

    float accumulator = 0.0f;
    const float STEP_SIZE = 1.0f / 60.0f;

    //Owned by physics system.
    physx::PxPhysics* gPhysics;
};

} // namespace physics