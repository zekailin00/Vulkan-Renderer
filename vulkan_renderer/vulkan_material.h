#pragma once

#include "material.h"

#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_uniform.h"

namespace renderer
{

struct MaterialUniform
{
    glm::vec3 albedo = {255, 255, 255};
    float metallic = 0.0f;
    float smoothness = 0.5f;

    float useAlbedoTex = 0.0f;
    float useMetallicTex = 0.0f;
    float useSmoothnessTex = 0.0f;
    float useNormalTex = 0.0f;
};

class VulkanMaterial: public Material
{

public:
    static std::shared_ptr<Material> BuildMaterial(MaterialProperties* prop);
    static std::shared_ptr<Material> GetDefaultMaterial();

    ~VulkanMaterial();

    MaterialProperties* GetProperties() override;
    void ResetProperties() override;

    void AddAlbedoTexture(std::shared_ptr<Texture> texture) override;
    void AddMetallicTexture(std::shared_ptr<Texture> texture) override;
    void AddSmoothnessTexture(std::shared_ptr<Texture> texture) override;
    void AddNormalTexture(std::shared_ptr<Texture> texture) override;

    void ResetAlbedoTexture() override;
    void ResetMetallicTexture() override;
    void ResetSmoothnessTexture() override;
    void ResetNormalTexture() override;

    VkDescriptorSet* GetDescriptorSet() {return &descriptorSet;}

private:
    void Destory();

private:
    VulkanDevice* vulkanDevice = nullptr;
    
    VulkanUniform uniform{};
    MaterialUniform* map = nullptr;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    static std::shared_ptr<VulkanMaterial> defaultMaterial;
};

} // namespace renderer