#include "light_component.h"

#include "entity.h"
#include "scene.h"
#include "serialization.h"
#include "scene_contexts.h"


namespace renderer
{

Component* LightInitializer::operator()(Entity* entity)
{
    LightComponent* component = new LightComponent();
    component->entity = entity;
    component->type = Component::Type::Light;
    component->technique = technique;

    component->properties.type = DIRECTIONAL_LIGHT;
    component->properties.color = {5, 5, 5};
    component->light = VulkanLight::BuildLight(component->properties);

    return component;
}

Component* LightDeserializer::operator()(Entity* entity, Json::Value& json)
{
    LightComponent* component = new LightComponent();
    component->entity = entity;
    component->type = Component::Type::Light;
    component->technique = technique;

    glm::vec4 color;
    DeserializeVec4(color, json["color"]);
    component->properties.type = DIRECTIONAL_LIGHT;
    component->properties.color = color;
    component->light = VulkanLight::BuildLight(component->properties);

    return component;
}

void LightComponent::Update(Timestep ts)
{
    light->SetTransform(entity->GetGlobalTransform());
    light->dirLight.color = glm::vec4(properties.color, 1.0f);
    technique->PushRendererData(light->dirLight);

    Scene* scene = entity->GetScene();
    std::shared_ptr<SceneContext> ctx;
    if ((ctx = scene->GetSceneContext(SceneContext::Type::RendererCtx)))
    {
        std::shared_ptr<SceneRendererContext> renderCtx =
            std::dynamic_pointer_cast<SceneRendererContext>(ctx);
    }
}

void LightComponent::Serialize(Json::Value& json)
{
    SerializeVec4(light->dirLight.color, json["color"]);
}

LightComponent::~LightComponent()
{
    light = nullptr; // free the smart pointer.
}

} // namespace renderer