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
        physicsCtx = std::make_shared(*system->NewContext());

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

    return component;
}

Component* DynamicBodyDeserializer::operator()(Entity* entity, Json::Value& json)
{

}

void DynamicBodyComponent::Update(Timestep ts)
{
    if (!dynamicBody->GetKinematic())
    {
        glm::mat4 transform;
        dynamicBody->GetGlobalTransform(transform);
        
        // set global??
    }
}

void DynamicBodyComponent::Serialize(Json::Value& json)
{

}

DynamicBodyComponent::~DynamicBodyComponent() 
{

}

} // namespace physics