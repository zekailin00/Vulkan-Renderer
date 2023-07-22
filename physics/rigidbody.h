#pragma once

#include <PxPhysicsAPI.h>

#include "collision_shape.h"
#include <glm/mat4x4.hpp>
#include <vector>


namespace physics
{

class PhysicsContext;
class CollisionShape;

typedef physx::PxActorType::Enum RigidbodyType;

class Rigidbody
{

public:
    Rigidbody(PhysicsContext* context, physx::PxRigidActor* gRigidActor):
        context(context), gRigidActor(gRigidActor) {}

    virtual ~Rigidbody();

    virtual void UpdateCenterOfMass() = 0;

    PhysicsContext* GetContext() {return context;}

    RigidbodyType GetRigidbodyType()
    {
        return gRigidActor->getType();
    }

    void GetGlobalTransform(glm::mat4& transform) const;
    void SetGlobalTransform(const glm::mat4& transform);

    CollisionShape* AttachShape(GeometryType geometryType);
    void DetachShape(CollisionShape* shape);
    CollisionShape* GetShape(unsigned int index) const;
    unsigned int GetNbShapes() const;
    unsigned int GetShapes(std::vector<CollisionShape*>& shapes) const;

protected:
    std::vector<CollisionShape*> collisionShapeList;
    PhysicsContext* context; // Owned by scene

private:
    physx::PxRigidActor* gRigidActor; // Cached
};

} // namespace physics