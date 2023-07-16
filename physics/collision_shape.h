#pragma once

#include <PxPhysicsAPI.h>
#include <glm/mat4x4.hpp>

namespace physics
{

typedef physx::PxBoxGeometry BoxGeometry;
typedef physx::PxSphereGeometry SphereGeometry;
typedef physx::PxCapsuleGeometry CapsuleGeometry;
typedef physx::PxPlaneGeometry PlaneGeometry;
typedef physx::PxGeometryType::Enum GeometryType;

class PhysicsContext;
class DynamicRigidbody;
class StaticRigidbody;

class CollisionShape
{

public:
    void SetLocalTransform(const glm::mat4& transform);
    void GetLocalTransform(glm::mat4& transform) const;

    void SetTrigger(bool isTrigger);
    bool GetTrigger() const;

    GeometryType GetGeometry();

    void SetDynamicFriction(float coef);
    float GetDynamicFriction() const;

    void SetStaticFriction(float coef);
    float GetStaticFriction() const;

    void SetRestitution(float rest);
    float GetRestitution() const;

private:
    CollisionShape(physx::PxShape* gShape);

    friend PhysicsContext;
    friend DynamicRigidbody;
    friend StaticRigidbody;

private:
    physx::PxShape* gShape;
};

} // namespace physics
