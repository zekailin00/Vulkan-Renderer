#pragma once 

#include "vulkan/vulkan.h"

#include <vector>
#include <cstddef>

class VulkanVertexbuffer
{
public:
    void* vertexData{nullptr};
    void* indexData{nullptr};

    void Initialize(VkDeviceSize indexBufferSize, VkDeviceSize vertexBufferSize);
    void Destroy();

    void* MapIndex();
    void UnmapIndex();
    void* MapVertex();
    void UnmapVertex();

    static VkPipelineVertexInputStateCreateInfo* GetVertexInputState();

    VkBuffer indexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory indexMemory{VK_NULL_HANDLE};
    VkBuffer vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory vertexMemory{VK_NULL_HANDLE};

private:
    VkDeviceSize vertexBufferSize;
    VkDeviceSize indexBufferSize;

    static VkPipelineVertexInputStateCreateInfo vertexInputState;
    static std::vector<VkVertexInputAttributeDescription> inputAttributeDesc;
    static VkVertexInputBindingDescription inputBindingDesc;
};