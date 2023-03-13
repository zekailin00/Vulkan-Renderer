#pragma once 

#include <vulkan/vulkan.h>
#include <string>

#include "texture.h"
#include "vk_primitives/vulkan_device.h"


namespace renderer
{

class VulkanTexture: public renderer::Texture
{

public:
    static std::shared_ptr<Texture> BuildTexture(TextureBuildInfo*);
    static std::shared_ptr<VulkanTexture> GetDefaultTexture();

    VkDescriptorImageInfo* GetDescriptor(
        VkImageLayout vkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    ~VulkanTexture() override;

    void CreateImage(
        VkExtent2D imageExtent, VkFormat colorFormat, 
        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    void CreateSampler(
        VkFilter minFilter = VK_FILTER_LINEAR,
        VkFilter magFilter = VK_FILTER_LINEAR,
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    void LoadImageFromFile(std::string filePath);
    VkImageView GetImageView() {return vkImageView;}
    VkImage GetImage() {return vkImage;}
    void Destroy();

private:
    VkImage vkImage = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;
    VkImageView vkImageView = VK_NULL_HANDLE;
    VkSampler vkSampler = VK_NULL_HANDLE;
    VkExtent2D imageExtent{};
    VkDescriptorImageInfo vkDecriptorInfo{};

    static std::shared_ptr<VulkanTexture> defaultTexture;

    VulkanDevice* vulkanDevice = nullptr; // Owned by VulkanRenderer
};

} // namespace renderer