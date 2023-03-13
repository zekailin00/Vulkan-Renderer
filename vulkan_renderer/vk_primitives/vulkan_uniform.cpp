#include "vulkan_uniform.h"

#include "validation.h"
#include "logger.h"

#include <vulkan/vulkan.h>

void VulkanUniform::Initialize(VulkanDevice* vulkanDevice, VkDeviceSize size)
{
    if (this->vulkanDevice != nullptr)
        Destroy();
    this->vulkanDevice = vulkanDevice;
    VkDevice vkDevice = vulkanDevice->vkDevice;
    vkDeviceSize = size;

    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = vkDeviceSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CHECK_VKCMD(vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &vkBuffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkDevice, vkBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkanDevice->GetMemoryTypeIndex(memRequirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkDeviceMemory));

    vkBindBufferMemory(vkDevice, vkBuffer, vkDeviceMemory, 0);
    vkMapMemory(vkDevice, vkDeviceMemory, 0, vkDeviceSize, 0, &data);
}

VkDescriptorBufferInfo* VulkanUniform::GetDescriptor()
{
    if (vulkanDevice == nullptr)
        Log::Write(Log::Level::Error, "Uniform is not initialized");

    bufferInfo.buffer = vkBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = vkDeviceSize;
    return &bufferInfo;
}

void* VulkanUniform::Map()
{
    return data;
}

void VulkanUniform::Destroy()
{
    if (vulkanDevice == nullptr)
        return;

    VkDevice& vkDevice = vulkanDevice->vkDevice;

    // FIXME: may need to pause all GPU operations before deallocation.

    vkUnmapMemory(vkDevice, vkDeviceMemory);
    vkDestroyBuffer(vkDevice, vkBuffer, nullptr);
    vkFreeMemory(vkDevice, vkDeviceMemory, nullptr);

    vkDeviceSize = 0;
    data = nullptr;
    vulkanDevice = nullptr;
}