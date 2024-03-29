#include "collision_shape.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "physics_context.h"
#include "dynamic_rigidbody.h"
#include "validation.h"


namespace physics
{

CollisionShape::CollisionShape(physx::PxShape* gShape)
{
    this->gShape = gShape;
}

CollisionShape::~CollisionShape()
{
    physx::PxMaterial* gMaterial;

    rigidbody->GetContext()->RemoveTrigger(this);
    gShape->getMaterials(&gMaterial, 1);
    gMaterial->release();
    gShape->release();
}

void CollisionShape::SetLocalTransform(const glm::mat4& transform)
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

    gShape->setLocalPose(pose);
    UpdateCenterOfMass();
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
        // rigidbody->GetContext()->RemoveTrigger(this);
        // Notes: When shape is not removed and trigger is disabled
        // eNOTIFY_TOUCH_LOST is received, but not eREMOVED_SHAPE_OTHER
        // so trigger does NOT need to be removed here.
    }
    UpdateCenterOfMass();
}

bool CollisionShape::GetTrigger() const
{
    return (gShape->getFlags() & physx::PxShapeFlag::eTRIGGER_SHAPE);
}

GeometryType CollisionShape::GetGeometryType() const
{
    return gShape->getGeometryType();
}

void CollisionShape::SetGeometry(const Geometry &geometry)
{
    gShape->setGeometry(geometry);
    UpdateCenterOfMass();
}

bool CollisionShape::GetBoxGeometry(BoxGeometry& geometry) const
{
    return gShape->getBoxGeometry(geometry);
}

bool CollisionShape::GetSphereGeometry(SphereGeometry& geometry) const
{
    return gShape->getSphereGeometry(geometry);
}

bool CollisionShape::GetCapsuleGeometry(CapsuleGeometry& geometry) const
{
    return gShape->getCapsuleGeometry(geometry);
}

bool CollisionShape::GetPlaneGeometry(PlaneGeometry& geometry) const
{
    return gShape->getPlaneGeometry(geometry);
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

void CollisionShape::UpdateCenterOfMass()
{
    physx::PxRigidActor* actor = gShape->getActor();
    ASSERT(actor != nullptr);

    if (actor->getType() == physx::PxActorType::eRIGID_DYNAMIC)
    {
        rigidbody->UpdateCenterOfMass();
    }
}

} // namespace physics
