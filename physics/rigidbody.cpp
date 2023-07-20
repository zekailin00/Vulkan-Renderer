#include "rigidbody.h"

#include "collision_shape.h"
#include "math_library.h"

namespace physics
{


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

bool Rigidbody::AttachShape(CollisionShape* shape)
{
    gRigidActor->attachShape(*(shape->gShape));
    collisionShapeList.push_back(shape);
    shape->rigidbody = this;

    UpdateCenterOfMass();
    return true;
}

void Rigidbody::DetachShape(CollisionShape* shape)
{
    for (auto begin = collisionShapeList.begin();
        begin != collisionShapeList.end(); begin++)
    {
        if (shape == *begin)
        {
            gRigidActor->detachShape(*(*begin)->gShape);
            delete *begin;
            collisionShapeList.erase(begin);
            break;
        }
    }
    
    UpdateCenterOfMass();
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
