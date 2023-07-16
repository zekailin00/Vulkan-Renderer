#pragma once

#include <PxPhysicsAPI.h>

#include <glm/mat4x4.hpp>

#include <vector>

namespace physics
{

class CollisionShape;

class DynamicRigidbody
{
public:
    struct RigidDynamicsProperties
    {
        bool isKinematic;
        bool isGravity = true;
        bool isSimulated = false;

        float mass;
        glm::vec3 inertia;
        float linearDamp;
        float angularDamp;
    };

public:
    DynamicRigidbody(physx::PxRigidDynamic* rigidbody);

    glm::mat4 GetGlobalTransform() const;
    void SetGlobalTransform(const glm::mat4& transform);

    bool AttachShape(CollisionShape* shape);
    void DetachShape(CollisionShape* shape);
    unsigned int GetNbShapes() const;
    unsigned int GetShapes(std::vector<CollisionShape*>& shapes) const;

    void SetGravity(bool isEnabled);
    void SetSimulation(bool isEnabled);

    void SetMass(float mass);
    float GetMass() const; 
    void SetMassSpaceInertiaTensor(const glm::vec3& inertia);
    glm::vec3 GetMassSpaceInertiaTensor() const;

    void SetLinearDamping(float linearDamp);
    float GetLinearDamping() const;
    void SetAngularDamping(float angularDamp);
    float GetAngularDamping() const;

    /**
     * @brief It is invalid to use this method
     * if the actor has not been added to a scene already or 
     * if PxActorFlag::eDISABLE_SIMULATION is set.
     * 
     * @param force 
     * @param mode 
     */
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
    bool GetKinematicTarget(glm::mat4& target) const;

private:
    physx::PxRigidDynamic* rigidbody;
};

} // namespace physics
