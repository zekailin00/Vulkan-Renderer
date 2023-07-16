#pragma once

#include <PxPhysicsAPI.h>
#include <glm/mat4x4.hpp>

namespace physics
{

/**
 * @brief defines geometries this way would have physx math
 * library exposed to the top layer. May not be the best way.
 * 
 */
typedef physx::PxGeometry Geometry; 
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

    GeometryType GetGeometryType() const;

    void SetGeometry(const Geometry &geometry);

    template<typename T>
    bool GetGeometry(GeometryType type, T& geometry)
    {
        return gShape->getGeometryT<T>(type, geometry);
    }

    bool GetBoxGeometry(BoxGeometry& geometry) const;
    bool GetSphereGeometry(SphereGeometry& geometry) const;
    bool GetCapsuleGeometry(CapsuleGeometry& geometry) const;
    bool GetPlaneGeometry(PlaneGeometry& geometry) const;


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
