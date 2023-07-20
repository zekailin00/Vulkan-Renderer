#pragma once

#include "collision_shape.h"
#include "physics_context.h"

#include "scene.h"
#include "serialization.h"

#include <memory>

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

static std::shared_ptr<PhysicsContext> GetPhysicsContext(
    PhysicsSystem* system, Scene* scene)
{
    std::shared_ptr<PhysicsContext> physicsCtx;

    if (!scene->GetSceneContext(SceneContext::Type::PhysicsCtx))
    {
        physicsCtx = std::shared_ptr<PhysicsContext>(system->NewContext());

        scene->SetSceneContext(
            SceneContext::Type::PhysicsCtx,
            std::dynamic_pointer_cast<SceneContext>(physicsCtx)
        );
    }
    else
    {
        physicsCtx = std::dynamic_pointer_cast<PhysicsContext>(
            scene->GetSceneContext(SceneContext::Type::PhysicsCtx)
        );
    }

    return physicsCtx;
}

} // namespace physics