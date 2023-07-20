#pragma once

#include "rigidbody.h"

#include <PxPhysicsAPI.h>

namespace physics
{

class StaticRigidbody: public Rigidbody
{

public:
    StaticRigidbody(PhysicsContext* context, physx::PxRigidStatic* rigidbody);
    ~StaticRigidbody() override;

    StaticRigidbody(const StaticRigidbody&) = delete;
    void operator=(const StaticRigidbody&) = delete;

    void UpdateCenterOfMass() override {}; // Not used by static rigidbody

private:
    physx::PxRigidStatic* gRigidStatic;
};

} // namespace physics
