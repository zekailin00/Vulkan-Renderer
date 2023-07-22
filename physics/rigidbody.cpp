#include "rigidbody.h"

#include "physics_context.h"
#include "collision_shape.h"
#include "math_library.h"

#include "logger.h"

namespace physics
{

Rigidbody::~Rigidbody()
{
    for (auto& c: collisionShapeList)
    {
        delete c;
    }

    collisionShapeList.clear();
}

void Rigidbody::GetGlobalTransform(glm::mat4& transform) const
{
    physx::PxTransform pose = gRigidActor->getGlobalPose();

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

void Rigidbody::SetGlobalTransform(const glm::mat4& transform)
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

    gRigidActor->setGlobalPose(pose);
}

CollisionShape* Rigidbody::AttachShape(GeometryType geometryType)
{
    CollisionShape* collisionShape =
        context->AddCollisionShape(geometryType);
    if (collisionShape)
    {
        collisionShape->rigidbody = this;
        gRigidActor->attachShape(*(collisionShape->gShape));
        collisionShapeList.push_back(collisionShape);

        UpdateCenterOfMass();
    }
    else
    {
        Logger::Write(
            "[Physics] Geometry type is not supported for this rigidbody.",
            Logger::Level::Warning, Logger::MsgType::Physics
        );
    }

    return collisionShape;
}

void Rigidbody::DetachShape(CollisionShape* shape)
{
    bool found = false;
    for (auto begin = collisionShapeList.begin();
        begin != collisionShapeList.end(); begin++)
    {
        if (shape == *begin)
        {
            gRigidActor->detachShape(*(*begin)->gShape);
            delete *begin;
            collisionShapeList.erase(begin);
            found = true;
            break;
        }
    }

    if (found)
    {
        UpdateCenterOfMass();
    }
    else
    {
        Logger::Write(
            "[Physics] DetachShape failed because not shape is found",
            Logger::Level::Warning, Logger::MsgType::Physics
        );
    }
}

CollisionShape* Rigidbody::GetShape(unsigned int index) const
{
    if (index < collisionShapeList.size())
    {
        return collisionShapeList[index];
    }

    return nullptr;
}

unsigned int Rigidbody::GetNbShapes() const
{
    return collisionShapeList.size();
}

unsigned int Rigidbody::GetShapes(
    std::vector<CollisionShape*>& shapes) const
{
    shapes = collisionShapeList;
    return collisionShapeList.size();
}

} // namespace physics
