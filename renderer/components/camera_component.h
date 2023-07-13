#pragma once

#include "component.h"

#include "vulkan_camera.h"
#include "render_technique.h"

#include <string>


namespace renderer
{

class VulkanRenderer;

class CameraInitializer
{
    RenderTechnique* technique;
    VulkanDevice* vulkanDevice;
    VulkanPipelineLayout* linePipelineLayout;

public:
    CameraInitializer(
        RenderTechnique* technique,
        VulkanDevice* vulkanDevice,
        VulkanPipelineLayout* linePipelineLayout):
    technique(technique),
    vulkanDevice(vulkanDevice),
    linePipelineLayout(linePipelineLayout) {}

    Component* operator()(Entity* entity);
};

class CameraDeserializer
{
    RenderTechnique* technique;
    VulkanDevice* vulkanDevice;
    VulkanPipelineLayout* linePipelineLayout;

public:
    CameraDeserializer(
        RenderTechnique* technique,
        VulkanDevice* vulkanDevice,
        VulkanPipelineLayout* linePipelineLayout):
    technique(technique),
    vulkanDevice(vulkanDevice),
    linePipelineLayout(linePipelineLayout) {}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct CameraComponent: public Component
{
    std::shared_ptr<VulkanCamera> camera;

    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~CameraComponent() override;

private:
    friend CameraInitializer;
    friend CameraDeserializer;

    RenderTechnique* technique;
};

} // namespace renderer