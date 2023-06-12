#pragma once

#include "vulkan/vulkan.h"

#include <vector>

class VulkanDevice
{
public:
    void Initialize(VkInstance vkInstance, std::vector<const char*> deviceExt);
    uint32_t GetMemoryTypeIndex(uint32_t memoryType, VkMemoryPropertyFlags memoryProperties);
    VkFormat GetDepthFormat();
    void Destroy();

private:
    void InitializePhysicalDevice();
    void InitializeLogicalDevice(
        std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME});

    void GetPhysicalDeviceInfo();
    uint32_t GetQueueFamilyIndex(VkQueueFlags vkQueueFlags);

public:
    VkInstance vkInstance; // Obtained from Vulkan Renderer
    VkPhysicalDevice vkPhysicalDevice; // Created by Vulkan Device
    VkDevice vkDevice; // Created by Vulkan Device

    uint32_t graphicsIndex;
    VkQueue graphicsQueue;

private: 
    std::vector<VkQueueFamilyProperties> vkQueueFamilyProperties;
    std::vector<VkExtensionProperties> vkExtensionProperties;
    VkPhysicalDeviceProperties vkProperties;
    VkPhysicalDeviceFeatures vkFeatures;
    VkPhysicalDeviceMemoryProperties vkMemoryProperties;
};