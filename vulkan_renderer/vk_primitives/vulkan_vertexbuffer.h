#pragma once 

#include "vulkan_device.h"

#include <vulkan/vulkan.h>
#include <vector>

class VulkanVertexbuffer
{
public:

    /**
     * Initialize a vulkan vertex buffer.
     * Can only be called after renderer is allocated.
     * When calling more than one time,
     * all the old resources are deallocated,
     * and new resources will be allocated again.
    */
    void Initialize(VulkanDevice* vulkanDevice, 
        VkDeviceSize indexBufferSize, VkDeviceSize vertexBufferSize);

    /**
     * Deallocate all resources.
     * Automatically called in destructor.
     * Called multiple times will not cause errors,
     * but all vulkanVertexbuffers have to be deallocated
     * before the renderer is deallocated.
    */
    void Destroy();

    /**
     * Get mapped index address.
    */
    void* MapIndex();

    /**
     * Get mapped vertex address.
    */
    void* MapVertex();


    static VkPipelineVertexInputStateCreateInfo* GetVertexInputState();

    VulkanVertexbuffer() = default;
    ~VulkanVertexbuffer() {Destroy();}

    VulkanVertexbuffer(const VulkanVertexbuffer&) = delete;
    VulkanVertexbuffer& operator=(VulkanVertexbuffer&) = delete;

    VkBuffer indexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory indexMemory{VK_NULL_HANDLE};
    VkBuffer vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory vertexMemory{VK_NULL_HANDLE};

private:
    VulkanDevice* vulkanDevice{nullptr};

    void* vertexData{nullptr};
    void* indexData{nullptr};
    VkDeviceSize vertexBufferSize{0};
    VkDeviceSize indexBufferSize{0};

    static VkPipelineVertexInputStateCreateInfo vertexInputState;
    static std::vector<VkVertexInputAttributeDescription> inputAttributeDesc;
    static VkVertexInputBindingDescription inputBindingDesc;
};