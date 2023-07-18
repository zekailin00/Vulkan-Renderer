#include "dynamic_rigidbody.h"

#include "collision_shape.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace physics
{

DynamicRigidbody::DynamicRigidbody(
    PhysicsContext* context, physx::PxRigidDynamic* rigidbody)
{
    this->context = context;
    this->rigidbody = rigidbody;
}

DynamicRigidbody::~DynamicRigidbody()
{
    PX_RELEASE(rigidbody);
}

void DynamicRigidbody::GetGlobalTransform(glm::mat4& transform) const
{
    physx::PxTransform pose = rigidbody->getGlobalPose();

    glm::quat quaternion;
    quaternion.x = pose.q.x;
    quaternion.y = pose.q.y;
    quaternion.z = pose.q.z;
    quaternion.w = pose.q.w;

    glm::vec3 position;
    position.x = pose.p.x;
    position.y = pose.p.y;
    position.z = pose.p.z;

    transform =
        glm::translate(glm::mat4(1.0f), position) *
        glm::toMat4(quaternion);
}

void DynamicRigidbody::SetGlobalTransform(const glm::mat4& transform)
{
    glm::vec3 _0, _1;
    glm::quat rotation;
    glm::vec3 translate;
    glm::vec4 _2;
    glm::decompose(
        transform, _0, rotation,
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

    rigidbody->setGlobalPose(pose);
}

bool DynamicRigidbody::DynamicRigidbody::AttachShape(CollisionShape* shape)
{
    rigidbody->attachShape(*(shape->gShape));
    collisionShapeList.push_back(shape);
    return true;
}

void DynamicRigidbody::DetachShape(CollisionShape* shape)
{
    for (auto begin = collisionShapeList.begin();
        begin != collisionShapeList.end(); begin++)
    {
        if (shape == *begin)
        {
            rigidbody->detachShape(*(*begin)->gShape);
            delete *begin;
            collisionShapeList.erase(begin);
            break;
        }
    }
}

unsigned int DynamicRigidbody::GetNbShapes() const
{
    return collisionShapeList.size();
}

unsigned int DynamicRigidbody::GetShapes(
    std::vector<CollisionShape*>& shapes) const
{
    shapes = collisionShapeList;
    return collisionShapeList.size();
}

void DynamicRigidbody::SetGravity(bool isEnabled)
{
    rigidbody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !isEnabled);
}

bool DynamicRigidbody::GetGravity()
{
    return !(rigidbody->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY);
}

void DynamicRigidbody::SetMass(float mass)
{
    rigidbody->setMass(mass);
}

float DynamicRigidbody::GetMass() const
{
    return rigidbody->getMass();
}

void DynamicRigidbody::SetMassSpaceInertiaTensor(const glm::vec3& inertia)
{
    physx::PxVec3 gInertia;
    gInertia.x = inertia.x;
    gInertia.y = inertia.y;
    gInertia.z = inertia.z;

    rigidbody->setMassSpaceInertiaTensor(gInertia);
}

glm::vec3 DynamicRigidbody::GetMassSpaceInertiaTensor() const
{
    physx::PxVec3 gInertia = rigidbody->getMassSpaceInertiaTensor();

    glm::vec3 inertia;
    inertia.x = gInertia.x;
    inertia.y = gInertia.y;
    inertia.z = gInertia.z;

    return inertia;
}

void DynamicRigidbody::SetLinearDamping(float linearDamp)
{
    rigidbody->setLinearDamping(linearDamp);
}

float DynamicRigidbody::GetLinearDamping() const
{
    return rigidbody->getLinearDamping();
}

void DynamicRigidbody::SetAngularDamping(float angularDamp)
{
    rigidbody->setAngularDamping(angularDamp);
}

float DynamicRigidbody::GetAngularDamping() const
{
    return rigidbody->getAngularDamping();
}

void DynamicRigidbody::AddForce(const glm::vec3& force)
{
    physx::PxVec3 gForce;
    gForce.x = force.x;
    gForce.y = force.y;
    gForce.z = force.z;

    rigidbody->addForce(gForce);
}

void DynamicRigidbody::AddTorque(const glm::vec3& torque)
{
    physx::PxVec3 gTorque;
    gTorque.x = torque.x;
    gTorque.y = torque.y;
    gTorque.z = torque.z;

    rigidbody->addForce(gTorque);
}

void DynamicRigidbody::ClearForce()
{
    rigidbody->clearForce();
}

void DynamicRigidbody::ClearTorque()
{
    rigidbody->clearTorque();
}

glm::vec3 DynamicRigidbody::GetLinearVelocity() const
{
    physx::PxVec3 gVelocity = rigidbody->getLinearVelocity();

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

    rigidbody->setLinearVelocity(gLinearVelocity);
}

glm::vec3 DynamicRigidbody::GetAngularVelocity() const
{
    physx::PxVec3 gAngularVelocity = rigidbody->getAngularVelocity();

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

    rigidbody->setAngularVelocity(gAngularVelocity);
}

void DynamicRigidbody::SetKinematic(bool isKinematic)
{
    rigidbody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
}

bool DynamicRigidbody::GetKinematic()
{
    return (rigidbody->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC);
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

    rigidbody->setKinematicTarget(pose);
}

} // namespace physics
