#include "dynamic_rigidbody.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace physics
{

DynamicRigidbody::DynamicRigidbody(
    PhysicsContext* context, physx::PxRigidDynamic* rigidbody):
    Rigidbody(context, rigidbody)
{
    this->gRigidDynamic = rigidbody;
    UpdateCenterOfMass();
}

DynamicRigidbody::~DynamicRigidbody()
{
    PX_RELEASE(gRigidDynamic);
}

void DynamicRigidbody::SetGravity(bool isEnabled)
{
    gRigidDynamic->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !isEnabled);
    if (isEnabled && !this->GetKinematic())
    {
        // Force non-kinematic rigidbody to wake up with zero force
        gRigidDynamic->addForce({0.0f, 0.0f, 0.0f}, physx::PxForceMode::eFORCE, true);
    }
}

bool DynamicRigidbody::GetGravity()
{
    return !(gRigidDynamic->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY);
}

void DynamicRigidbody::UpdateCenterOfMass()
{
    physx::PxRigidBodyExt::updateMassAndInertia(
        *gRigidDynamic, properties.density
    );
    WakeUp();
}

void DynamicRigidbody::WakeUp()
{
    if (GetKinematic())
    {
        return;
    }

    gRigidDynamic->wakeUp();
}

void DynamicRigidbody::SetDensity(float density)
{
    if (density == 0.0f)
    {
        return;
    }

    properties.density = density;
    UpdateCenterOfMass();
}

float DynamicRigidbody::GetDensity() const
{
    return properties.density;
}

void DynamicRigidbody::SetMass(float mass)
{
    gRigidDynamic->setMass(mass);
    UpdateCenterOfMass();
}

float DynamicRigidbody::GetMass() const
{
    return gRigidDynamic->getMass();
}

void DynamicRigidbody::SetMassSpaceInertiaTensor(const glm::vec3& inertia)
{
    physx::PxVec3 gInertia;
    gInertia.x = inertia.x;
    gInertia.y = inertia.y;
    gInertia.z = inertia.z;

    gRigidDynamic->setMassSpaceInertiaTensor(gInertia);
    UpdateCenterOfMass();
}

glm::vec3 DynamicRigidbody::GetMassSpaceInertiaTensor() const
{
    physx::PxVec3 gInertia = gRigidDynamic->getMassSpaceInertiaTensor();

    glm::vec3 inertia;
    inertia.x = gInertia.x;
    inertia.y = gInertia.y;
    inertia.z = gInertia.z;

    return inertia;
}

void DynamicRigidbody::SetLinearDamping(float linearDamp)
{
    gRigidDynamic->setLinearDamping(linearDamp);
}

float DynamicRigidbody::GetLinearDamping() const
{
    return gRigidDynamic->getLinearDamping();
}

void DynamicRigidbody::SetAngularDamping(float angularDamp)
{
    gRigidDynamic->setAngularDamping(angularDamp);
}

float DynamicRigidbody::GetAngularDamping() const
{
    return gRigidDynamic->getAngularDamping();
}

void DynamicRigidbody::AddForce(const glm::vec3& force)
{
    physx::PxVec3 gForce;
    gForce.x = force.x;
    gForce.y = force.y;
    gForce.z = force.z;

    gRigidDynamic->addForce(gForce);
}

void DynamicRigidbody::AddTorque(const glm::vec3& torque)
{
    physx::PxVec3 gTorque;
    gTorque.x = torque.x;
    gTorque.y = torque.y;
    gTorque.z = torque.z;

    gRigidDynamic->addTorque(gTorque);
}

void DynamicRigidbody::ClearForce()
{
    gRigidDynamic->clearForce();
}

void DynamicRigidbody::ClearTorque()
{
    gRigidDynamic->clearTorque();
}

glm::vec3 DynamicRigidbody::GetLinearVelocity() const
{
    physx::PxVec3 gVelocity = gRigidDynamic->getLinearVelocity();

    glm::vec3 velocity;
    velocity.x = gVelocity.x;
    velocity.y = gVelocity.y;
    velocity.z = gVelocity.z;

    return velocity;
}

void DynamicRigidbody::SetLinearVelocity(const glm::vec3& linearVelocity)
{
    physx::PxVec3 gLinearVelocity;
    gLinearVelocity.x = linearVelocity.x;
    gLinearVelocity.y = linearVelocity.y;
    gLinearVelocity.z = linearVelocity.z;

    gRigidDynamic->setLinearVelocity(gLinearVelocity);
}

glm::vec3 DynamicRigidbody::GetAngularVelocity() const
{
    physx::PxVec3 gAngularVelocity = gRigidDynamic->getAngularVelocity();

    glm::vec3 angularVelocity;
    angularVelocity.x = gAngularVelocity.x;
    angularVelocity.y = gAngularVelocity.y;
    angularVelocity.z = gAngularVelocity.z;

    return angularVelocity;
}

void DynamicRigidbody::SetAngularVelocity(const glm::vec3& angularVelocity)
{
    physx::PxVec3 gAngularVelocity;
    gAngularVelocity.x = angularVelocity.x;
    gAngularVelocity.y = angularVelocity.y;
    gAngularVelocity.z = angularVelocity.z;

    gRigidDynamic->setAngularVelocity(gAngularVelocity);
}

void DynamicRigidbody::SetKinematic(bool isKinematic)
{
    gRigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
    if (!isKinematic)
    {
        // Force non-kinematic rigidbody to wake up with zero force
        gRigidDynamic->addForce({0.0f, 0.0f, 0.0f}, physx::PxForceMode::eFORCE, true);
    }
}

bool DynamicRigidbody::GetKinematic()
{
    return (gRigidDynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC);
}

void DynamicRigidbody::SetKinematicTarget(const glm::mat4& destination)
{
    glm::vec3 _0, _1;
    glm::quat rotation;
    glm::vec3 translate;
    glm::vec4 _2;
    glm::decompose(
        destination, _0, rotation,
        translate, _1, _2
    );

    physx::PxTransform pose;
    pose.p.x = translate.x;
    pose.p.y = translate.y;
    pose.p.z = translate.z;

    pose.q.w = rotation.w;
    pose.q.x = rotation.x;
    pose.q.y = rotation.y;
    pose.q.z = rotation.z;

    gRigidDynamic->setKinematicTarget(pose);
}

} // namespace physics
