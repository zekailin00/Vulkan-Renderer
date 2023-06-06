#pragma once

#include <vulkan/vulkan.h>

#include "vk_primitives/vulkan_device.h"


struct VulkanUniform
{
public:

    /**
     * Initialize uniform and vulkan resources are allocated.
     * If it is called multiple times,
     * previously allocated resources are destroyed.
    */
    void Initialize(VulkanDevice* vulkanDevice, VkDeviceSize size);

    /**
     * Destroy all vulkan resources.
     * Reset the object to the uninitialized state.
     * Can be called multiple times.
    */
    void Destroy();

    /**
     * @brief Get the memory mapped address of the GPU buffer.
     * The mapping is host visible and host coherent.
     * Return nullptr if uniform is not initialized.
     * 
     * @return void* 
     */
    void* Map();

    VkBufferView GetBufferView(); // Not used?

    /**
     * Get Descriptor buffer info.
     * If uniform is not initialized, error is thrown.
    */
    VkDescriptorBufferInfo* GetDescriptor();

    VulkanUniform() = default;
    ~VulkanUniform() {Destroy();}

    VulkanUniform(const VulkanUniform&) = delete;
    VulkanUniform& operator=(const VulkanUniform&) = delete;

    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
    VkDeviceSize vkDeviceSize;
    VkDescriptorBufferInfo bufferInfo{};

private: // Access through methods
    VulkanDevice* vulkanDevice = nullptr; // Owned by renderer
    void* data = nullptr;
};