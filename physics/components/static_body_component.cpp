#include "components/static_body_component.h"

#include "physics_context.h"
#include "physics_system.h"

#include "static_rigidbody.h"
#include "components/physics_components_common.h"

#include "serialization.h"
#include "math_library.h"

#include "entity.h"
#include "scene.h"
#include <memory>


namespace physics
{


Component* StaticBodyInitializer::operator()(Entity* entity)
{
    StaticBodyComponent* component = new StaticBodyComponent();
    component->entity = entity;
    component->type = Component::Type::StaticBody;

    Scene* scene = entity->GetScene();
    std::shared_ptr<PhysicsContext> physicsCtx = GetPhysicsContext(system, scene);

    component->staticBody = physicsCtx->NewStaticRigidbody();
    component->staticBody->SetGlobalTransform(
        entity->GetGlobalTransform()
    );

    return component;
}

Component* StaticBodyDeserializer::operator()(Entity* entity, Json::Value& json)
{
    StaticBodyComponent* component = new StaticBodyComponent();
    component->entity = entity;
    component->type = Component::Type::StaticBody;

    Scene* scene = entity->GetScene();
    std::shared_ptr<PhysicsContext> physicsCtx = GetPhysicsContext(system, scene);

    component->staticBody = physicsCtx->NewStaticRigidbody();
    component->staticBody->SetGlobalTransform(
        entity->GetGlobalTransform()
    );

    Json::Value& jsonShapeList = json["shapeList"];
    for (int i = 0; i < jsonShapeList.size(); i++)
    {
        Json::Value& jsonShape = jsonShapeList[i];

        switch (jsonShape["GeometryType"].asInt())
        {
        case GeometryType::eBOX:
        {
            CollisionShape* shape = component->staticBody->AttachShape(
                GeometryType::eBOX
            );

            glm::vec3 halfExtent;
            DeserializeVec3(halfExtent, jsonShape["halfExtent"]);

            BoxGeometry box;
            shape->GetBoxGeometry(box);
            box.halfExtents.x = halfExtent.x;
            box.halfExtents.y = halfExtent.y;
            box.halfExtents.z = halfExtent.z;
            shape->SetGeometry(box);

            DeserializeShapeCommons(shape, jsonShape);
        }
            break;

        case GeometryType::eSPHERE:
        {
            CollisionShape* shape = component->staticBody->AttachShape(
                GeometryType::eSPHERE
            );

            SphereGeometry sphere;
            shape->GetSphereGeometry(sphere);
            sphere.radius = jsonShape["radius"].asFloat();
            shape->SetGeometry(sphere);

            DeserializeShapeCommons(shape, jsonShape);
        }
            break;

        case GeometryType::eCAPSULE:
        {
            CollisionShape* shape = component->staticBody->AttachShape(
                GeometryType::eCAPSULE
            );

            CapsuleGeometry capsule;
            shape->GetCapsuleGeometry(capsule);
            capsule.halfHeight = jsonShape["halfHeight"].asFloat();
            capsule.radius = jsonShape["radius"].asFloat();
            shape->SetGeometry(capsule);

            DeserializeShapeCommons(shape, jsonShape);
        }
            break;

        default:
            throw;
        }
    }

    return component;
}

void StaticBodyComponent::Update(Timestep ts)
{
    if (entity->GetScene()->GetSceneContext(SceneContext::Type::RendererCtx))
    {
        std::shared_ptr<SceneRendererContext> renderCtx =
            std::dynamic_pointer_cast<SceneRendererContext>(
                entity->GetScene()->GetSceneContext(SceneContext::Type::RendererCtx)
            );
        // debug lines
        std::vector<CollisionShape*> shapes;
        staticBody->GetShapes(shapes);
        for(auto& shape: shapes)
        {
            switch (shape->GetGeometryType())
            {
            case GeometryType::eBOX:
            {
                glm::mat4 tf = entity->GetGlobalTransformNoScale();
                glm::mat4 shapeTf;
                shape->GetLocalTransform(shapeTf);

                BoxGeometry box;
                shape->GetBoxGeometry(box);
                glm::vec3 scale = 
                {
                    box.halfExtents.x * 2,
                    box.halfExtents.y * 2,
                    box.halfExtents.z * 2
                };

                glm::mat4 shapeScaleTf = glm::scale(glm::mat4(1.0f), scale);

                glm::mat4 finalTransform = tf * shapeTf * shapeScaleTf;

                renderCtx->RenderDebugOBB(finalTransform);

            }    
                break;

            case GeometryType::eSPHERE:
            {
                glm::mat4 tf = entity->GetGlobalTransformNoScale();
                glm::mat4 shapeTf;
                shape->GetLocalTransform(shapeTf);

                glm::mat4 finalTransform = tf * shapeTf;

                SphereGeometry sphere;
                shape->GetSphereGeometry(sphere);

                renderCtx->RenderDebugSphere(
                    glm::vec3(finalTransform[3]),
                    sphere.radius
                );
            }
                break;

            case GeometryType::eCAPSULE:
            {
                glm::mat4 tf = entity->GetGlobalTransformNoScale();
                glm::mat4 shapeTf;
                shape->GetLocalTransform(shapeTf);

                glm::mat4 finalTransform = tf * shapeTf;

                CapsuleGeometry capsule;
                shape->GetCapsuleGeometry(capsule);

                renderCtx->RenderDebugCapsule(
                    capsule.halfHeight,
                    capsule.radius,
                    finalTransform
                );
            }
                break;

            default:
                throw;
            }
        }
    }
}

void StaticBodyComponent::Serialize(Json::Value& json)
{
    std::vector<CollisionShape*> shapeList;
    staticBody->GetShapes(shapeList);

    Json::Value& jsonShapeList = json["shapeList"];
    for (int i = 0; i < shapeList.size(); i++)
    {
        CollisionShape* shape = shapeList[i];
        Json::Value& jsonShape = jsonShapeList[i];

        switch (shape->GetGeometryType())
        {
        case GeometryType::eBOX:
        {
            jsonShape["GeometryType"] = GeometryType::eBOX;

            BoxGeometry box;
            shape->GetBoxGeometry(box);

            glm::vec3 halfExtent =
            {
                box.halfExtents.x,
                box.halfExtents.y,
                box.halfExtents.z
            };
            SerializeVec3(halfExtent, jsonShape["halfExtent"]);

            SerializeShapeCommons(shape, jsonShape);
        }
            break;

        case GeometryType::eSPHERE:
        {
            jsonShape["GeometryType"] = GeometryType::eSPHERE;

            SphereGeometry sphere;
            shape->GetSphereGeometry(sphere);
            jsonShape["radius"] = sphere.radius;

            SerializeShapeCommons(shape, jsonShape);
        }
            break;

        case GeometryType::eCAPSULE:
        {
            jsonShape["GeometryType"] = GeometryType::eCAPSULE;

            CapsuleGeometry capsule;
            shape->GetCapsuleGeometry(capsule);
            jsonShape["halfHeight"] = capsule.halfHeight;
            jsonShape["radius"] = capsule.radius;

            SerializeShapeCommons(shape, jsonShape);
        }
            break;

        default:
            throw;
        }
    }
}

StaticBodyComponent::~StaticBodyComponent() 
{
    delete staticBody;
}

} // namespace physics
