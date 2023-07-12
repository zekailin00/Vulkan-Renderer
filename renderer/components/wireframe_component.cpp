#include "wireframe_component.h"

#include "entity.h"
#include "serialization.h" 


namespace renderer
{

Component* LineInitializer::operator()(Entity* entity)
{
    LineComponent* component = new LineComponent();
    component->entity = entity;
    component->type = Component::Type::Wireframe;
    component->technique = technique;

    component->lineRenderer = std::make_shared<LineRenderer>(
        vulkanDevice, linePipelineLayout);

    return component;
}

Component* LineDeserializer::operator()(Entity* entity, Json::Value& json)
{
    LineComponent* component = new LineComponent();
    component->entity = entity;
    component->type = Component::Type::Wireframe;
    component->technique = technique;

    VulkanWireframe::WireframeType wireframeType =
        (VulkanWireframe::WireframeType)json["type"].asInt();

    glm::vec3 color;
    DeserializeVec3(color, json["color"]);
    float width = json["width"].asFloat();

    return component;
}

void LineComponent::Update(Timestep ts)
{   
    LineRenderer::LineProperties* prop = lineRenderer->GetLineProperties();
    prop->model = entity->GetGlobalTransform();
    technique->PushRendererData(lineRenderer);
}

void LineComponent::Serialize(Json::Value& json)
{
    // json["type"]  = wireframe->type;
    // SerializeVec3(wireframe->color, json["color"]);
    // json["width"] = wireframe->width;

}

LineComponent::~LineComponent()
{
    lineRenderer = nullptr; // free the smart pointer
}

} // namespace renderer