#pragma once

#include "material.h"

#include "vk_primitives/vulkan_device.h"
#include "vk_primitives/vulkan_uniform.h"

namespace renderer
{

struct MaterialUniform
{
    glm::vec4 albedo = {1, 1, 1, 0};

    float metallic = 0.1f;
    float roughness = 0.9f;
    float _1;
    float _2;

    float useAlbedoTex = 0.0f;
    float useMetallicTex = 0.0f;
    float useRoughnessTex = 0.0f;
    float useNormalTex = 0.0f;
};

class VulkanMaterial: public Material
{

public:
    static std::shared_ptr<Material> BuildMaterial(MaterialProperties* prop);
    static std::shared_ptr<Material> GetDefaultMaterial();
    static void DestroyDefaultMaterial();

    ~VulkanMaterial();

    const MaterialProperties* GetProperties() override;
    void ResetProperties() override;

    void SetAlbedo(glm::vec3 albedo) override;
    void SetMetallic(float metallic) override;
    void SetRoughness(float roughness) override;

    void AddAlbedoTexture(std::shared_ptr<Texture> texture) override;
    void AddMetallicTexture(std::shared_ptr<Texture> texture) override;
    void AddRoughnessTexture(std::shared_ptr<Texture> texture) override;
    void AddNormalTexture(std::shared_ptr<Texture> texture) override;

    void ResetAlbedoTexture() override;
    void ResetMetallicTexture() override;
    void ResetRoughnessTexture() override;
    void ResetNormalTexture() override;

    void Serialize(Json::Value& json) override;

    VkDescriptorSet* GetDescriptorSet() {return &descriptorSet;}

private:
    void Destory();

private:
    VulkanDevice* vulkanDevice = nullptr;
    
    MaterialProperties properties;
    VulkanUniform uniform{};
    MaterialUniform* map = nullptr;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    std::string resourcePath;

    static std::shared_ptr<VulkanMaterial> defaultMaterial;
};

} // namespace renderer