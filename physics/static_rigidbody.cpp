#include "static_rigidbody.h"


namespace physics
{

StaticRigidbody::StaticRigidbody(
    PhysicsContext* context, physx::PxRigidStatic* rigidbody):
    Rigidbody(context, rigidbody)
{
    this->gRigidStatic = rigidbody;
}

StaticRigidbody::~StaticRigidbody()
{
    PX_RELEASE(gRigidStatic);
}

} // namespace physics
