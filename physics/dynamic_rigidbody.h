#pragma once

#include "rigidbody.h"

#include <PxPhysicsAPI.h>
#include <glm/mat4x4.hpp>
#include <vector>

namespace physics
{

class PhysicsContext;
class CollisionShape;

class DynamicRigidbody: public Rigidbody
{
public:
    struct RigidDynamicsProperties
    {
        bool isKinematic;
        bool isGravity = true;

        float density = 10.0f;
        float linearDamp;
        float angularDamp;
    };

public:
    DynamicRigidbody(PhysicsContext* context, physx::PxRigidDynamic* rigidbody);
    ~DynamicRigidbody() override;

    DynamicRigidbody(const DynamicRigidbody&) = delete;
    void operator=(const DynamicRigidbody&) = delete;

    void SetGravity(bool isEnabled);
    bool GetGravity();

    void UpdateCenterOfMass() override;
    void WakeUp();

    void SetDensity(float density);
    float GetDensity() const;

    void SetMass(float mass);
    float GetMass() const; 
    void SetMassSpaceInertiaTensor(const glm::vec3& inertia);
    glm::vec3 GetMassSpaceInertiaTensor() const;

    void SetLinearDamping(float linearDamp);
    float GetLinearDamping() const;
    void SetAngularDamping(float angularDamp);
    float GetAngularDamping() const;

    void AddForce(const glm::vec3& force);
    void AddTorque(const glm::vec3& torque);
    void ClearForce();
    void ClearTorque();

    glm::vec3 GetLinearVelocity() const;
    void SetLinearVelocity(const glm::vec3& linearVelocity);
    glm::vec3 GetAngularVelocity() const;
    void SetAngularVelocity(const glm::vec3& angularVelocity);

    void SetKinematic(bool isKinematic);
    bool GetKinematic();
    void SetKinematicTarget(const glm::mat4& destination);

private:
    physx::PxRigidDynamic* gRigidDynamic;
    RigidDynamicsProperties properties{};
};

} // namespace physics
