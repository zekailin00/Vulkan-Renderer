#pragma once

#include "component.h"

#include "vulkan_wireframe.h"
#include "render_technique.h"

#include <string>


namespace renderer
{

class WireframeInitializer
{
    RenderTechnique* technique;

public:
    WireframeInitializer(RenderTechnique* technique)
    :technique(technique){}

    Component* operator()(Entity* entity);
};

class WireframeDeserializer
{
    RenderTechnique* technique;

public:
    WireframeDeserializer(RenderTechnique* technique)
    :technique(technique){}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct WireframeComponent: public Component
{
    std::shared_ptr<VulkanWireframe> wireframe;
    
    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~WireframeComponent() override;

private:
    friend WireframeInitializer;
    friend WireframeDeserializer;

    RenderTechnique* technique;
};

} // namespace renderer