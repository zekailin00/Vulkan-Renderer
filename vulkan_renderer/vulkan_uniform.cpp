#include "vulkan_uniform.h"

#include "vulkan_renderer.h"
#include "validation.h"
#include "logger.h"

#include "vulkan/vulkan.h"

void VulkanUniform::Initialize(VkDeviceSize size)
{
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;
    VkDevice vkDevice = vulkanDevice.vkDevice;
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
    allocInfo.memoryTypeIndex = vulkanDevice.GetMemoryTypeIndex(memRequirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkDeviceMemory));

    vkBindBufferMemory(vkDevice, vkBuffer, vkDeviceMemory, 0);
}

VkDescriptorBufferInfo VulkanUniform::GetDescriptor()
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = vkBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = vkDeviceSize;
    return bufferInfo;
}

void* VulkanUniform::Map()
{
    if (data) return data;

    VkDevice& vkDevice = VulkanRenderer::GetInstance().vulkanDevice.vkDevice;
    vkMapMemory(vkDevice, vkDeviceMemory, 0, vkDeviceSize, 0, &data);
    return data;
}

void VulkanUniform::Unmap()
{
    if (data)
    {
        data = nullptr;
        VkDevice& vkDevice = VulkanRenderer::GetInstance().vulkanDevice.vkDevice;
        vkUnmapMemory(vkDevice, vkDeviceMemory);
    }
}

void VulkanUniform::Destroy()
{
    VkDevice& vkDevice = VulkanRenderer::GetInstance().vulkanDevice.vkDevice;
    vkDestroyBuffer(vkDevice, vkBuffer, nullptr);
    vkFreeMemory(vkDevice, vkDeviceMemory, nullptr);
}