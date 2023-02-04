#include "vulkan_vertexbuffer.h"

#include "vulkan_renderer.h"
#include "pipeline_inputs.h"
#include "validation.h"


VkPipelineVertexInputStateCreateInfo VulkanVertexbuffer::vertexInputState;
std::vector<VkVertexInputAttributeDescription> VulkanVertexbuffer::inputAttributeDesc;
VkVertexInputBindingDescription VulkanVertexbuffer::inputBindingDesc;

void VulkanVertexbuffer::Initialize(VkDeviceSize indexBufferSize, VkDeviceSize vertexBufferSize)
{
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;
    VkDevice vkDevice = vulkanDevice.vkDevice;
    
    this->indexBufferSize = indexBufferSize;
    this->vertexBufferSize = vertexBufferSize;

    {
        VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size = indexBufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        CHECK_VKCMD(vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &indexBuffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vkDevice, indexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkanDevice.GetMemoryTypeIndex(memRequirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &indexMemory));

        vkBindBufferMemory(vkDevice, indexBuffer, indexMemory, 0);
    }

    {
        VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size = vertexBufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        CHECK_VKCMD(vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &vertexBuffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vkDevice, vertexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkanDevice.GetMemoryTypeIndex(memRequirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vertexMemory));

        vkBindBufferMemory(vkDevice, vertexBuffer, vertexMemory, 0);
    }
}

void* VulkanVertexbuffer::MapIndex()
{
    if (indexData) return indexData;
    
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;
    vkMapMemory(vulkanDevice.vkDevice, indexMemory, 0, indexBufferSize, 0, &indexData);

    return indexData;
}
    
void VulkanVertexbuffer::UnmapIndex()
{
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;
    vkUnmapMemory(vulkanDevice.vkDevice, indexMemory);
    indexData = nullptr;
}

void* VulkanVertexbuffer::MapVertex()
{
    if (vertexData) return vertexData;

    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;
    vkMapMemory(vulkanDevice.vkDevice, vertexMemory, 0, vertexBufferSize, 0, &vertexData);

    return vertexData;
}
    
void VulkanVertexbuffer::UnmapVertex()
{
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;
    vkUnmapMemory(vulkanDevice.vkDevice, vertexMemory);
    vertexData = nullptr;
}

void VulkanVertexbuffer::Destroy()
{
    VkDevice& vkDevice = VulkanRenderer::GetInstance().vulkanDevice.vkDevice;

    vkDestroyBuffer(vkDevice, indexBuffer, nullptr);
    vkFreeMemory(vkDevice, indexMemory, nullptr);

    vkDestroyBuffer(vkDevice, vertexBuffer, nullptr);
    vkFreeMemory(vkDevice, vertexMemory, nullptr);
}

VkPipelineVertexInputStateCreateInfo* VulkanVertexbuffer::GetVertexInputState()
{
    inputBindingDesc.binding = 0;
    inputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    inputBindingDesc.stride = sizeof(Vertex);

    inputAttributeDesc = 
    {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position)},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Normal)},
        {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, TexCoords)},
    };

    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &inputBindingDesc;
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributeDesc.size());
    vertexInputState.pVertexAttributeDescriptions = inputAttributeDesc.data();

    return &vertexInputState;
}