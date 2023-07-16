#pragma once

#include <PxPhysicsAPI.h>
#include <glm/mat4x4.hpp>

namespace physics
{

class PhysicsContext;
class DynamicRigidbody;
class StaticRigidbody;

class CollisionShape
{

public:
    enum class Geometry
    {
        Box,
        Sphere,
        Capsule,
        Plane
    };

public:
    void SetLocalTransform(const glm::mat4& transform);
    void GetLocalTransform(glm::mat4& transform) const;

    void SetTrigger(bool isTrigger);
    bool GetTrigger() const;

    Geometry GetGeometry();

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
