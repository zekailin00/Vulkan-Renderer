#include "ui_component.h"

namespace renderer
{

Component* UIInitializer::operator()(Entity* entity)
{
    UIComponent* component = new UIComponent();
    component->entity = entity;
    component->type = Component::Type::UI;
    component->technique = technique;

    component->ui = nullptr;

    return component;
}

Component* UIDeserializer::operator()(Entity* entity, Json::Value& json)
{
    throw; //FIXME: does not support yet
    return nullptr;
}

void UIComponent::Update(Timestep ts)
{
    if (!ui)
        return;
    
    technique->PushRendererData(ui);
}

void UIComponent::Serialize(Json::Value& json)
{
    throw; //FIXME: does not support yet.
}

UIComponent::~UIComponent()
{
    ui = nullptr; // free the smart pointer.
}

} // namespace renderer
