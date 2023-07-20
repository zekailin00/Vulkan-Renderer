#pragma once

#include "collision_shape.h"
#include "serialization.h"

namespace physics
{

static void SerializeShapeCommons(CollisionShape* shape, Json::Value& jsonShape)
{
    glm::mat4 transform;
    shape->GetLocalTransform(transform);
    SerializeMat4(transform, jsonShape["transform"]);

    jsonShape["staticFriction"]  = shape->GetStaticFriction();
    jsonShape["dynamicFriction"] = shape->GetDynamicFriction();
    jsonShape["restitution"]     = shape->GetRestitution();
    jsonShape["isTrigger"]       = shape->GetTrigger();
}

static void DeserializeShapeCommons(CollisionShape* shape, Json::Value& jsonShape)
{
    glm::mat4 transform;
    DeserializeMat4(transform, jsonShape["transform"]);
    shape->SetLocalTransform(transform);

    shape->SetStaticFriction(jsonShape["staticFriction"].asFloat());
    shape->SetDynamicFriction(jsonShape["dynamicFriction"].asFloat());
    shape->SetRestitution(jsonShape["restitution"].asFloat());
    shape->SetTrigger(jsonShape["isTrigger"].asBool());
}

} // namespace physics