#include "components/dynamic_body_component.h"

#include "physics_context.h"
#include "physics_system.h"

#include "dynamic_rigidbody.h"
#include "components/physics_components_common.h"

#include "serialization.h"
#include "math_library.h"

#include "entity.h"
#include "scene.h"
#include <memory>

namespace physics
{

Component* DynamicBodyInitializer::operator()(Entity* entity)
{
    DynamicBodyComponent* component = new DynamicBodyComponent();
    component->entity = entity;
    component->type = Component::Type::DynamicBody;

    Scene* scene = entity->GetScene();
    std::shared_ptr<PhysicsContext> physicsCtx = GetPhysicsContext(system, scene);

    component->dynamicBody = physicsCtx->NewDynamicRigidbody();
    component->dynamicBody->SetGlobalTransform(
        entity->GetGlobalTransform()
    );

    return component;
}

Component* DynamicBodyDeserializer::operator()(Entity* entity, Json::Value& json)
{
    DynamicBodyComponent* component = new DynamicBodyComponent();
    component->entity = entity;
    component->type = Component::Type::DynamicBody;

    Scene* scene = entity->GetScene();
    std::shared_ptr<PhysicsContext> physicsCtx = GetPhysicsContext(system, scene);

    component->dynamicBody = physicsCtx->NewDynamicRigidbody();
    component->dynamicBody->SetGlobalTransform(
        entity->GetGlobalTransform()
    );

    component->dynamicBody->SetKinematic(json["isKinematic"].asBool());
    component->dynamicBody->SetGravity(json["isGravity"].asBool());
    component->dynamicBody->SetDensity(json["density"].asFloat());
    component->dynamicBody->SetLinearDamping(json["linearDamp"].asFloat());
    component->dynamicBody->SetAngularDamping(json["angularDamp"].asFloat());

    Json::Value& jsonShapeList = json["shapeList"];
    for (int i = 0; i < jsonShapeList.size(); i++)
    {
        Json::Value& jsonShape = jsonShapeList[i];

        switch (jsonShape["GeometryType"].asInt())
        {
        case GeometryType::eBOX:
        {
            CollisionShape* shape = component->dynamicBody->AttachShape(
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
            CollisionShape* shape = component->dynamicBody->AttachShape(
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
            CollisionShape* shape = component->dynamicBody->AttachShape(
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

void DynamicBodyComponent::Update(Timestep ts)
{
    glm::mat4 transform;
    dynamicBody->GetGlobalTransform(transform);
    
    glm::mat4 parentTransform = entity->GetParent()->GetGlobalTransform();
    glm::mat4 localTransform = glm::inverse(parentTransform) * transform;
    glm::mat4 scaleTransform = glm::scale(glm::mat4(1.0f), entity->GetLocalScale());
    entity->SetLocalTransform(localTransform * scaleTransform, true);

    if (entity->GetScene()->GetSceneContext(SceneContext::Type::RendererCtx))
    {
        std::shared_ptr<SceneRendererContext> renderCtx =
            std::dynamic_pointer_cast<SceneRendererContext>(
                entity->GetScene()->GetSceneContext(SceneContext::Type::RendererCtx)
            );
        // debug lines
        std::vector<CollisionShape*> shapes;
        dynamicBody->GetShapes(shapes);
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

void DynamicBodyComponent::Serialize(Json::Value& json)
{
    //TODO: for now, mass and inertia are automatically
    // computed based on density and shapes' volume

    json["isKinematic"] = dynamicBody->GetKinematic();
    json["isGravity"] = dynamicBody->GetGravity();

    json["density"] = dynamicBody->GetDensity();
    json["linearDamp"] = dynamicBody->GetLinearDamping();
    json["angularDamp"] = dynamicBody->GetAngularDamping();

    std::vector<CollisionShape*> shapeList;
    dynamicBody->GetShapes(shapeList);

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

DynamicBodyComponent::~DynamicBodyComponent() 
{
    delete dynamicBody;
}

} // namespace physics