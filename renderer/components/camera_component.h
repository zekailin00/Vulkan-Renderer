#pragma once

#include "component.h"

#include "vulkan_Camera.h"
#include "render_technique.h"

#include <string>


namespace renderer
{

class VulkanRenderer;

class CameraInitializer
{
    RenderTechnique* technique;
    VulkanRenderer* renderer;

public:
    CameraInitializer(RenderTechnique* technique, VulkanRenderer* renderer)
    :technique(technique), renderer(renderer){}

    Component* operator()(Entity* entity);
};

class CameraDeserializer
{
    RenderTechnique* technique;
    VulkanRenderer* renderer;

public:
    CameraDeserializer(RenderTechnique* technique, VulkanRenderer* renderer)
    :technique(technique), renderer(renderer){}

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