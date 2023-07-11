#pragma once

#include "mesh.h"

#include "vulkan_material.h"
#include "vk_primitives/vulkan_vertexbuffer.h"
#include "vk_primitives/vulkan_uniform.h"
#include "vk_primitives/vulkan_buffer.h"

#include <memory>


namespace renderer
{

class VulkanMesh: public Mesh
{

public:
    static std::shared_ptr<Mesh> BuildMesh(BuildMeshInfo& info);

    void AddMaterial(std::shared_ptr<Material> material) override;

    void RemoveMaterial() override;

    void Serialize(Json::Value& json) override;

    std::string GetResourcePath() override;

    ~VulkanMesh() override;

    VulkanVertexbuffer& GetVertexbuffer();
    std::shared_ptr<VulkanMaterial> GetVulkanMaterial();

private:
    VulkanVertexbuffer vertexbuffer{};
    // VulkanUniform uniform{};
    // glm::mat4* map = nullptr;
    // VkDescriptorSet descSet = VK_NULL_HANDLE;

    std::shared_ptr<Material> material;
    
    std::string resourcePath;
};

class VulkanInstanceMesh: public Mesh
{

public:
    static std::shared_ptr<Mesh> BuildMesh(BuildMeshInfo& info);

    void AddMaterial(std::shared_ptr<Material> material) override;

    void RemoveMaterial() override;

    void Serialize(Json::Value& json) override;

    std::string GetResourcePath() override;

    ~VulkanInstanceMesh() override;

    VulkanVertexbuffer& GetVertexbuffer();
    std::shared_ptr<VulkanMaterial> GetVulkanMaterial();

private:
    VulkanVertexbuffer vertexbuffer{};
    std::shared_ptr<Material> material;


    VulkanBuffer<glm::mat4>* transformBuffer;
    std::string resourcePath;

};
} // namespace renderer