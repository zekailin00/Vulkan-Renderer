#pragma once

#include <PxPhysicsAPI.h>
#include <glm/mat4x4.hpp>
#include <functional>


class Entity;

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
class Rigidbody;
class CollisionShape;

struct TriggerEvent
{
    Entity* triggerEntity;
    CollisionShape* triggerCollisionShape;
    Entity* otherEntity;
    CollisionShape* otherCollisionShape;

    bool operator==(const TriggerEvent& other) const
    {
        return (triggerCollisionShape == other.triggerCollisionShape) &&
               (otherCollisionShape == other.otherCollisionShape);
    }
};

class CollisionShape
{

public:
    void SetLocalTransform(const glm::mat4& transform);
    void GetLocalTransform(glm::mat4& transform) const;

    void SetTrigger(bool isTrigger);
    bool GetTrigger() const;

    void SetOnTriggerEnter(std::function<void(TriggerEvent*)> callback)
    {
        OnTriggerEnter = callback;
    }

    void SetOnTriggerStay(std::function<void(TriggerEvent*)> callback)
    {
        OnTriggerStay = callback;
    }

    void SetOnTriggerLeave(std::function<void(TriggerEvent*)> callback)
    {
        OnTriggerLeave = callback;
    }

    void ClearOnTriggerEnter()
    {
        OnTriggerEnter = nullptr;
    }

    void ClearOnTriggerStay()
    {
        OnTriggerStay = nullptr;
    }

    void ClearOnTriggerLeave()
    {
        OnTriggerLeave = nullptr;
    }

    GeometryType GetGeometryType() const;

    void SetGeometry(const Geometry &geometry);

    template<typename T>
    bool GetGeometry(GeometryType type, T& geometry) const
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

    ~CollisionShape();

private:
    CollisionShape(physx::PxShape* gShape);

    void UpdateCenterOfMass();

    void ExecuteOnTriggerEnter(TriggerEvent* event)
    {
        if (OnTriggerEnter)
            OnTriggerEnter(event);
    }

    void ExecuteOnTriggerStay(TriggerEvent* event)
    {
        if (OnTriggerStay)
            OnTriggerStay(event);
    }

    void ExecuteOnTriggerLeave(TriggerEvent* event)
    {
        if (OnTriggerLeave)
            OnTriggerLeave(event);
    }

    friend PhysicsContext;
    friend Rigidbody;

private:
    physx::PxShape* gShape; // storage

    std::function<void(TriggerEvent*)> OnTriggerEnter = nullptr;
    std::function<void(TriggerEvent*)> OnTriggerLeave = nullptr;
    std::function<void(TriggerEvent*)> OnTriggerStay = nullptr;

    Rigidbody* rigidbody; // owned by component
};

} // namespace physics
