#pragma once

#include "component.h"

#include "vulkan_light.h"
#include "render_technique.h"

namespace renderer
{

class LightInitializer
{
    RenderTechnique* technique;

public:
    LightInitializer(RenderTechnique* technique)
    :technique(technique){}

    Component* operator()(Entity* entity);
};

class LightDeserializer
{
    RenderTechnique* technique;

public:
    LightDeserializer(RenderTechnique* technique)
    :technique(technique){}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct LightComponent: public Component
{
    LightProperties properties;
    std::shared_ptr<VulkanLight> light;

    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~LightComponent() override;

private:
    friend LightInitializer;
    friend LightDeserializer;

    RenderTechnique* technique;
};

} // namespace renderer