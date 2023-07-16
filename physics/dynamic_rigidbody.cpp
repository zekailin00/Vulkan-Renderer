#include "dynamic_rigidbody.h"


namespace physics
{

DynamicRigidbody::DynamicRigidbody(physx::PxRigidDynamic* rigidbody)
{
    this->rigidbody = rigidbody;
}

} // namespace physics
