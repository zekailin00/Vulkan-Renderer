#include "collision_shape.h"

#include <glm/gtx/quaternion.hpp>

#include "validation.h"


namespace physics
{

CollisionShape::CollisionShape(physx::PxShape* shape)
{
    this->gShape = gShape;
}

void CollisionShape::SetLocalTransform(const glm::mat4& transform)
{
    glm::quat quaternion = glm::toQuat(transform);

    physx::PxTransform pose;
    pose.p.x = transform[3][0];
    pose.p.y = transform[3][1];
    pose.p.z = transform[3][2];

    pose.q.w = quaternion.w;
    pose.q.x = quaternion.x;
    pose.q.y = quaternion.y;
    pose.q.z = quaternion.z;

    gShape->setLocalPose(pose);
}

void CollisionShape::GetLocalTransform(glm::mat4& transform) const
{
    physx::PxTransform pose = gShape->getLocalPose();

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

void CollisionShape::SetTrigger(bool isTrigger)
{
    if (isTrigger)
    {
        gShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, false);
        gShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
        gShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
    }
    else
    {
        gShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
        gShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        gShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
    }
}

bool CollisionShape::GetTrigger() const
{
    return (gShape->getFlags() | physx::PxShapeFlag::eTRIGGER_SHAPE);
}

CollisionShape::Geometry CollisionShape::GetGeometry()
{
    physx::PxGeometryType::Enum type = gShape->getGeometryType();

    switch (type)
    {
    case physx::PxGeometryType::eBOX:
        return CollisionShape::Geometry::Box;
        break;

    case physx::PxGeometryType::eSPHERE:
        return CollisionShape::Geometry::Sphere;
        break;

    case physx::PxGeometryType::eCAPSULE:
        return CollisionShape::Geometry::Capsule;
        break;

    case physx::PxGeometryType::ePLANE:
        return CollisionShape::Geometry::Plane;
        break;
    
    default:
        throw;
    }
}

void CollisionShape::SetDynamicFriction(float coef)
{
    ASSERT((gShape->getNbMaterials() == 1));
    physx::PxMaterial* gMaterial;
    gShape->getMaterials(&gMaterial, 1, 0);
    gMaterial->setDynamicFriction(coef);
}

float CollisionShape::GetDynamicFriction() const
{
    ASSERT((gShape->getNbMaterials() == 1));
    physx::PxMaterial* gMaterial;
    gShape->getMaterials(&gMaterial, 1, 0);
    return gMaterial->getDynamicFriction();
}

void CollisionShape::SetStaticFriction(float coef)
{
    ASSERT((gShape->getNbMaterials() == 1));
    physx::PxMaterial* gMaterial;
    gShape->getMaterials(&gMaterial, 1, 0);
    gMaterial->setStaticFriction(coef);
}

float CollisionShape::GetStaticFriction() const
{
    ASSERT((gShape->getNbMaterials() == 1));
    physx::PxMaterial* gMaterial;
    gShape->getMaterials(&gMaterial, 1, 0);
    return gMaterial->getStaticFriction();
}

void CollisionShape::SetRestitution(float rest)
{
    ASSERT((gShape->getNbMaterials() == 1));
    physx::PxMaterial* gMaterial;
    gShape->getMaterials(&gMaterial, 1, 0);
    gMaterial->setRestitution(rest);
}

float CollisionShape::GetRestitution() const
{
    ASSERT((gShape->getNbMaterials() == 1));
    physx::PxMaterial* gMaterial;
    gShape->getMaterials(&gMaterial, 1, 0);
    return gMaterial->getRestitution();
}

} // namespace physics
