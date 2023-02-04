#pragma once 

#include "vulkan/vulkan.h"

#include <string>

struct VulkanTextureColor2D
{
public:
    void CreateImage(VkExtent2D imageExtent, VkFormat colorFormat, 
                     VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    void CreateSampler(VkFilter minFilter = VK_FILTER_LINEAR,
                       VkFilter magFilter = VK_FILTER_LINEAR);
    void LoadImageFromFile(std::string filePath);
    void Destroy();

    VkDescriptorImageInfo GetDescriptor(VkImageLayout vkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    VkImage vkImage = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;
    VkImageView vkImageView = VK_NULL_HANDLE;
    VkSampler vkSampler = VK_NULL_HANDLE;
    VkExtent2D imageExtent{};

private: // Access through method
    VkDescriptorImageInfo vkDecriptorInfo;
};

class VulkanTextureCubeMap
{

};