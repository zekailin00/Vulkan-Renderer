#pragma once

#include <PxPhysicsAPI.h>

namespace physics
{

class PhysicsContext
{

public:
    PhysicsContext(physx::PxPhysics* gPhysics);
    ~PhysicsContext();

private:
    physx::PxDefaultCpuDispatcher* gDispatcher;
    physx::PxScene* gScene;
};

} // namespace physics