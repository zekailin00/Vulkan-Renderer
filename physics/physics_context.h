#pragma once

#include <PxPhysicsAPI.h>

#include "collision_shape.h"
#include "timestep.h"
#include "scene_contexts.h"

#include <list>


namespace physics
{

class StaticRigidbody;
class DynamicRigidbody;
class Rigidbody;
class PhysicsContext;

class SimulationEventCallback: public physx::PxSimulationEventCallback
{
public:
    SimulationEventCallback(PhysicsContext* context):
        context(context) {}

private:
	void onConstraintBreak(
        physx::PxConstraintInfo* /*constraints*/, physx::PxU32 /*count*/) {}

	void onWake(physx::PxActor** /*actors*/, physx::PxU32 /*count*/) {}

	void onSleep(physx::PxActor** /*actors*/, physx::PxU32 /*count*/) {}

	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);

	void onAdvance(
        const physx::PxRigidBody*const*,
        const physx::PxTransform*, const physx::PxU32) {}

	void onContact(
        const physx::PxContactPairHeader& /*pairHeader*/,
        const physx::PxContactPair* pairs, physx::PxU32 count) {}

private:
    PhysicsContext* context;
};

class PhysicsContext: public ScenePhysicsContext
{

public:
    PhysicsContext(physx::PxPhysics* gPhysics);
    ~PhysicsContext();

    void AddTriggerEvent(TriggerEvent& event);
    void RemoveTriggerEvent(TriggerEvent& event);
    void RemoveTrigger(CollisionShape* shape);

    StaticRigidbody* NewStaticRigidbody();
    DynamicRigidbody* NewDynamicRigidbody();

    int Simulate(Timestep ts) override;

    void UpdatePhysicsTransform(Entity* e) override;

private:
    void RemoveRigidbody(physx::PxRigidActor* actor);
    CollisionShape* AddCollisionShape(GeometryType geometryType);
    void ProcessOnTriggerStayEvents();

    friend StaticRigidbody;
    friend DynamicRigidbody;
    friend Rigidbody;

private:
    SimulationEventCallback* simulationEventCallback;
    physx::PxDefaultCpuDispatcher* gDispatcher;
    physx::PxScene* gScene;

    float accumulator = 0.0f;
    const float STEP_SIZE = 1.0f / 60.0f;

    std::list<TriggerEvent> triggerEvents;

    //Owned by physics system.
    physx::PxPhysics* gPhysics;
};

} // namespace physics