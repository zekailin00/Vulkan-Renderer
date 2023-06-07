#pragma once

#include "component.h"

#include "vulkan_mesh.h"
#include "render_technique.h"

#include <string>


namespace renderer
{

class MeshInitializer
{
    RenderTechnique* technique;

public:
    MeshInitializer(RenderTechnique* technique)
    :technique(technique){}

    Component* operator()(Entity* entity);
};

class MeshDeserializer
{
    RenderTechnique* technique;

public:
    MeshDeserializer(RenderTechnique* technique)
    :technique(technique){}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct MeshComponent: public Component
{

    std::shared_ptr<VulkanMesh> mesh;

    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~MeshComponent() override;

private:
    friend MeshInitializer;
    friend MeshDeserializer;

    RenderTechnique* technique;
    std::shared_ptr<VulkanMesh> Mesh;
};

} // namespace renderer