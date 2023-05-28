#include "vulkan_device.h"

#include "validation.h"
#include "logger.h"

#include <iostream>
#include <vector>
#include <string>

#include <tracy/Tracy.hpp>


void VulkanDevice::Initialize(VkInstance vkInstance)
{
    ZoneScopedN("VulkanDevice::Initialize");

    this->vkInstance = vkInstance;
    InitializePhysicalDevice();
    InitializeLogicalDevice();
}

uint32_t VulkanDevice::GetMemoryTypeIndex(uint32_t memoryType, VkMemoryPropertyFlags memoryProperties)
{
    ZoneScopedN("VulkanDevice::GetMemoryTypeIndex");
    
    for (uint32_t i = 0; i < vkMemoryProperties.memoryTypeCount; i++) 
    {
        if ((memoryType & (1 << i)) && (vkMemoryProperties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties) 
            return i;
    }

    Log::Write(Log::Level::Error, "[Vulkan Device] Failed to find suitable memory type.");
    exit(1);
}

void VulkanDevice::InitializePhysicalDevice()
{
    ZoneScopedN("VulkanDevice::InitializePhysicalDevice");

    uint32_t gpuCount;
    CHECK_VKCMD(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, NULL));
    VkPhysicalDevice* gpuList = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpuCount);
    CHECK_VKCMD(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, gpuList));

    int gpuIndex = 0;
    for (int i = 0; i < static_cast<int>(gpuCount); i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(gpuList[i], &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            gpuIndex = i;
            break;
        }
    }

    vkPhysicalDevice = gpuList[gpuIndex];
    free(gpuList);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &properties);
    std::cout << "[Vulkan Renderer] Device [" << gpuIndex << "] : " << properties.deviceName << std::endl;

    GetPhysicalDeviceInfo();
}

void VulkanDevice::InitializeLogicalDevice(std::vector<const char*> extensions)
{
    ZoneScopedN("VulkanDevice::InitializeLogicalDevice");

    const float queuePriority = 0.0f;
    VkDeviceQueueCreateInfo vkQueueCreateInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    vkQueueCreateInfo.queueFamilyIndex = graphicsIndex = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
    vkQueueCreateInfo.queueCount = 1;
    vkQueueCreateInfo.pQueuePriorities = &queuePriority;

    //TODO: check all extensions are supported.
    // Enable portability for Apple support if it is included.
    for(auto extensionProperty: vkExtensionProperties)
    {
        if (std::strcmp(extensionProperty.extensionName, "VK_KHR_portability_subset") == 0)
            extensions.push_back("VK_KHR_portability_subset");
    }


    VkDeviceCreateInfo vkDeviceCreateInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkQueueCreateInfo;
    vkDeviceCreateInfo.enabledExtensionCount = (uint32_t)extensions.size();
    vkDeviceCreateInfo.ppEnabledExtensionNames = extensions.data();

    // Create logical device and device queues.
    CHECK_VKCMD(vkCreateDevice(vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &vkDevice));
    vkGetDeviceQueue(vkDevice, graphicsIndex, 0, &graphicsQueue);
}

void VulkanDevice::GetPhysicalDeviceInfo()
{
    ZoneScopedN("VulkanDevice::GetPhysicalDeviceInfo");

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
    vkQueueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, vkQueueFamilyProperties.data());

    uint32_t extensionsCount;
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionsCount, nullptr);
    vkExtensionProperties.resize(extensionsCount);
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionsCount, vkExtensionProperties.data());

    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkProperties);
    vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vkFeatures);
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkMemoryProperties);
}

uint32_t VulkanDevice::GetQueueFamilyIndex(VkQueueFlags vkQueueFlags)
{
    ZoneScopedN("VulkanDevice::GetQueueFamilyIndex");

    for (uint32_t i = 0; i < vkQueueFamilyProperties.size(); i++)
        if((vkQueueFamilyProperties[i].queueFlags & vkQueueFlags) == vkQueueFlags) return i;
    
    Log::Write(Log::Level::Error, "[Vulkan Device] Error, no matching queue family indices found.");
    exit(1);
}

VkFormat VulkanDevice::GetDepthFormat()
{
    ZoneScopedN("VulkanDevice::GetDepthFormat");

    return VK_FORMAT_D32_SFLOAT_S8_UINT;
}

void VulkanDevice::Destroy()
{
    ZoneScopedN("VulkanDevice::Destroy");

    vkDestroyDevice(vkDevice, nullptr);
}