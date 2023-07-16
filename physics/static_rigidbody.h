#pragma once

#include <PxPhysicsAPI.h>

namespace physics
{

class StaticRigidbody
{

public:
    StaticRigidbody(physx::PxScene* gScene);

private:
    physx::PxRigidStatic* gRigidStatic;
};

} // namespace physics
