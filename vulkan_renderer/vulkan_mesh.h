#pragma once

#include "mesh.h"

#include "vulkan_material.h"
#include "vk_primitives/vulkan_vertexbuffer.h"
#include "vk_primitives/vulkan_uniform.h"

#include <memory>


namespace renderer
{

class VulkanMesh: public Mesh
{

public:
    static std::shared_ptr<Mesh> BuildMesh(BuildMeshInfo& info);

    void AddMaterial(std::shared_ptr<Material> material) override;

    void RemoveMaterial() override;

    ~VulkanMesh() override;

private:
    VulkanVertexbuffer vertexbuffer{};
    VulkanUniform uniform{};
    glm::mat4* map = nullptr;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
};

} // namespace renderer