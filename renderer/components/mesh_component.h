#pragma once

#include "component.h"

#include "vulkan_mesh.h"
#include "render_technique.h"

#include <string>


namespace renderer
{

class VulkanRenderer;

class MeshInitializer
{
    RenderTechnique* technique;
    VulkanRenderer* renderer;

public:
    MeshInitializer(RenderTechnique* technique, VulkanRenderer* renderer)
    :technique(technique), renderer(renderer){}

    Component* operator()(Entity* entity);
};

class MeshDeserializer
{
    RenderTechnique* technique;
    VulkanRenderer* renderer;

public:
    MeshDeserializer(RenderTechnique* technique, VulkanRenderer* renderer)
    :technique(technique), renderer(renderer){}

    Component* operator()(Entity* entity, Json::Value& json);
};

struct MeshComponent: public Component
{
    std::shared_ptr<VulkanMesh> mesh;

    VulkanUniform uniform{};
    VkDescriptorSet descSet = VK_NULL_HANDLE;
    VulkanDevice* vulkanDevice = nullptr;
    glm::mat4* transform = nullptr;

    void Update(Timestep ts) override;
    void Serialize(Json::Value& json) override;
    ~MeshComponent() override;

private:
    friend MeshInitializer;
    friend MeshDeserializer;

    RenderTechnique* technique;
};

} // namespace renderer