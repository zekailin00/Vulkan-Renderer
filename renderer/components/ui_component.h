#pragma once

#include "component.h"

#include "vulkan_ui.h"
#include "render_technique.h"

#include <string>


namespace renderer
{

class UIInitializer
{
    RenderTechnique* technique;

public:
    UIInitializer(RenderTechnique* technique)
    :technique(technique){}

    Component* operator()(Entity* entity);
};

class UIDeserializer
{
    RenderTechnique* technique;

public:
    UIDeserializer(RenderTechnique* technique)
    :technique(technique){}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct UIComponent: public Component
{
    std::shared_ptr<VulkanUI> ui;
    std::shared_ptr<VulkanTexture> uiTexture;


    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~UIComponent() override;

private:
    friend UIInitializer;
    friend UIDeserializer;

    RenderTechnique* technique;
    std::shared_ptr<VulkanUI> UI;
};

} // namespace renderer