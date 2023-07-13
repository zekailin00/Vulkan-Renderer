#include "wireframe_component.h"

#include "entity.h"
#include "serialization.h" 


namespace renderer
{

Component* LineInitializer::operator()(Entity* entity)
{
    LineComponent* component = new LineComponent();
    component->entity = entity;
    component->type = Component::Type::Line;
    component->technique = technique;

    component->lineRenderer = std::make_shared<LineRenderer>(
        vulkanDevice, linePipelineLayout);

    return component;
}

Component* LineDeserializer::operator()(Entity* entity, Json::Value& json)
{
    LineComponent* component = new LineComponent();
    component->entity = entity;
    component->type = Component::Type::Line;
    component->technique = technique;

    component->lineRenderer = std::make_shared<LineRenderer>(
        vulkanDevice, linePipelineLayout);
    
    LineRenderer::LineProperties* prop =
        component->lineRenderer->GetLineProperties();
    DeserializeVec3(prop->color, json["color"]);
    prop->width = json["width"].asFloat();
    prop->useGlobalTransform = json["useGlobalTransform"].asInt();

    Json::Value& jsonLineData = json["lineData"];
    for(int i = 0; i < jsonLineData.size(); i++)
    {
        renderer::LineData lineData;
        DeserializeVec3(lineData.beginPoint, jsonLineData[i]["beginPoint"]);
        DeserializeVec3(lineData.endPoint, jsonLineData[i]["endPoint"]);

        component->lineRenderer->AddLine(lineData);
    }

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
    LineRenderer::LineProperties* prop = lineRenderer->GetLineProperties();
    SerializeVec3(prop->color, json["color"]);
    json["width"] = prop->width;
    json["useGlobalTransform"] = prop->useGlobalTransform;

    const renderer::LineData* lineData = lineRenderer->GetLineData()->Data();

    Json::Value& jsonLineData = json["lineData"];
    for(int i = 0; i < lineRenderer->GetLineData()->Size(); i++)
    {
        SerializeVec3(lineData[i].beginPoint, jsonLineData[i]["beginPoint"]);
        SerializeVec3(lineData[i].endPoint, jsonLineData[i]["endPoint"]);
    }
}

LineComponent::~LineComponent()
{
    lineRenderer = nullptr; // free the smart pointer
}

} // namespace renderer