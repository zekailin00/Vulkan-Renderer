#pragma once

#include "vulkan/vulkan.h"

struct VulkanUniform
{
public:
    void Initialize(VkDeviceSize size);
    void Destroy();

    /**
     * @brief Get the memory mapped address of the GPU buffer.
     * The mapping is host visible and host coherent.
     * 
     * @return void* 
     */
    void* Map();
    void Unmap();

    VkBufferView GetBufferView();
    VkDescriptorBufferInfo GetDescriptor();
    
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
    VkDeviceSize vkDeviceSize;

private: // Access through methods
    VkDescriptorBufferInfo vkDecriptorInfo; 
    void* data = nullptr;
};