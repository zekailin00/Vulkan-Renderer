#include "components/dynamic_body_component.h"

#include "physics_context.h"
#include "physics_system.h"

#include "dynamic_rigidbody.h"

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

    component->dynamicBody = physicsCtx->NewDynamicRigidbody();

    component->dynamicBody->SetGlobalTransform(
        entity->GetGlobalTransform()
    );

    return component;
}

Component* DynamicBodyDeserializer::operator()(Entity* entity, Json::Value& json)
{
    return nullptr;
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
        std::vector<physics::CollisionShape*> shapes;
        dynamicBody->GetShapes(shapes);
        for(auto& shape: shapes)
        {
            switch (shape->GetGeometryType())
            {
            case physics::GeometryType::eBOX:
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

            case physics::GeometryType::eSPHERE:
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

            case physics::GeometryType::eCAPSULE:
                /* code */
                break;

            case physics::GeometryType::ePLANE:
                /* code */
                throw;
                break;
            
            default:
                break;
            }
        }
    }
}

void DynamicBodyComponent::Serialize(Json::Value& json)
{

}

DynamicBodyComponent::~DynamicBodyComponent() 
{
    delete dynamicBody;
}

} // namespace physics