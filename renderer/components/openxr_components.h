#pragma once

#include "component.h"

#include "vulkan_camera.h"
#include "render_technique.h"

namespace renderer
{

class VrDisplayInitializer
{

public:
    VrDisplayInitializer(RenderTechnique* technique)
        :technique(technique){}

    RenderTechnique* technique;

    Component* operator()(Entity* entity);
};

class VrDisplayDeserializer
{

public:
    VrDisplayDeserializer(RenderTechnique* technique)
        :technique(technique){}

    RenderTechnique* technique;

    Component* operator()(Entity* entity, Json::Value& json);
};

struct VrDisplayComponent: public Component
{
    std::shared_ptr<VulkanVrDisplay> vrDisplay;
    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~VrDisplayComponent() override;

private:
    friend VrDisplayInitializer;
    friend VrDisplayDeserializer;

    RenderTechnique* technique;
};

} // namespace renderer