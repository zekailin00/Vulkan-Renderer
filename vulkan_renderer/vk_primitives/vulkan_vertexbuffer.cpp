#include "vulkan_vertexbuffer.h"

#include "pipeline_inputs.h"
#include "validation.h"


VkPipelineVertexInputStateCreateInfo VulkanVertexbuffer::vertexInputState;
std::vector<VkVertexInputAttributeDescription> VulkanVertexbuffer::inputAttributeDesc;
VkVertexInputBindingDescription VulkanVertexbuffer::inputBindingDesc;

void VulkanVertexbuffer::Initialize(VulkanDevice* vulkanDevice,
    VkDeviceSize indexBufferSize, VkDeviceSize vertexBufferSize)
{
    if (this->vulkanDevice != nullptr)
        Destroy();
    
    this->vulkanDevice = vulkanDevice;
    VkDevice vkDevice = vulkanDevice->vkDevice;
    
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
        allocInfo.memoryTypeIndex = vulkanDevice->GetMemoryTypeIndex(
                memRequirements.memoryTypeBits, 
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
        allocInfo.memoryTypeIndex = vulkanDevice->GetMemoryTypeIndex(
            memRequirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        CHECK_VKCMD(vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vertexMemory));

        vkBindBufferMemory(vkDevice, vertexBuffer, vertexMemory, 0);
    }

    vkMapMemory(vkDevice, vertexMemory, 0, vertexBufferSize, 0, &vertexData);
    vkMapMemory(vkDevice, indexMemory, 0, indexBufferSize, 0, &indexData);
}

void* VulkanVertexbuffer::MapIndex()
{
    return indexData;
}

void* VulkanVertexbuffer::MapVertex()
{
    return vertexData;
}

void VulkanVertexbuffer::Destroy()
{
    if (vulkanDevice == nullptr)
        return;
    
    VkDevice& vkDevice = vulkanDevice->vkDevice;

    vkUnmapMemory(vulkanDevice->vkDevice, indexMemory);
    vkUnmapMemory(vulkanDevice->vkDevice, vertexMemory);

    vkDestroyBuffer(vkDevice, indexBuffer, nullptr);
    vkFreeMemory(vkDevice, indexMemory, nullptr);

    vkDestroyBuffer(vkDevice, vertexBuffer, nullptr);
    vkFreeMemory(vkDevice, vertexMemory, nullptr);

    this->indexBufferSize = 0;
    this->vertexBufferSize = 0;
    indexData = nullptr;
    vertexData = nullptr;
    vulkanDevice = nullptr;
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
    vertexInputState.vertexAttributeDescriptionCount = inputAttributeDesc.size();
    vertexInputState.pVertexAttributeDescriptions = inputAttributeDesc.data();

    return &vertexInputState;
}