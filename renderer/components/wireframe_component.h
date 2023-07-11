#pragma once

#include "component.h"

#include "vulkan_wireframe.h"
#include "render_technique.h"
#include "vk_primitives/vulkan_device.h"

#include <string>


namespace renderer
{

class LineInitializer
{
    RenderTechnique* technique;
    VulkanDevice* vulkanDevice;

public:
    LineInitializer(RenderTechnique* technique, VulkanDevice* vulkanDevice)
    :technique(technique), vulkanDevice(vulkanDevice) {}

    Component* operator()(Entity* entity);
};

class LineDeserializer
{
    RenderTechnique* technique;
    VulkanDevice* vulkanDevice;

public:
    LineDeserializer(RenderTechnique* technique, VulkanDevice* vulkanDevice)
    :technique(technique), vulkanDevice(vulkanDevice) {}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct LineComponent: public Component
{
    std::shared_ptr<LineRenderer> lineRenderer;
    
    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~LineComponent() override;

private:
    friend LineInitializer;
    friend LineDeserializer;

    RenderTechnique* technique;
};

} // namespace renderer