#include "light_component.h"

#include "entity.h"
#include"serialization.h" 


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