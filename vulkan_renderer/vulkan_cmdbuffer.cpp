#include "vulkan_cmdbuffer.h"

#include "vulkan_renderer.h"
#include "validation.h"
#include "logger.h"

#include <vector>

void VulkanCmdBuffer::Initialize(uint32_t frame_in_flight)
{
    this->frame_in_flight = frame_in_flight;
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;

    VkCommandPoolCreateInfo vkCommandPoolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    vkCommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolInfo.queueFamilyIndex = vulkanDevice.graphicsIndex;
    CHECK_VKCMD(vkCreateCommandPool(vulkanDevice.vkDevice, &vkCommandPoolInfo, nullptr, &vkCommandPool));
    CHECK_VKCMD(vkCreateCommandPool(vulkanDevice.vkDevice, &vkCommandPoolInfo, nullptr, &singleCommandPool));

    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = frame_in_flight;
    vkCommandBuffers.resize(frame_in_flight);
    CHECK_VKCMD(vkAllocateCommandBuffers(vulkanDevice.vkDevice, &vkCommandBufferAllocateInfo, vkCommandBuffers.data()));

    CreateSynchPrimitives();
}

void VulkanCmdBuffer::Destroy()
{
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;
    
    vkDestroyCommandPool(vulkanDevice.vkDevice, vkCommandPool, nullptr);
    vkDestroyCommandPool(vulkanDevice.vkDevice, singleCommandPool, nullptr);
    DestroySynchPrimitives();
}

void VulkanCmdBuffer::CreateSynchPrimitives()
{
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;

    VkSemaphoreCreateInfo vkSemaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo vkFenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    vkFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    imageAcquiredSemaphores.resize(frame_in_flight);
    renderFinishedSemaphores.resize(frame_in_flight);
    queueSubmissionFences.resize(frame_in_flight);
    for (uint32_t i = 0; i < frame_in_flight; i++)
    {
        CHECK_VKCMD(vkCreateSemaphore(vulkanDevice.vkDevice, &vkSemaphoreInfo, nullptr, &imageAcquiredSemaphores[i]));
        CHECK_VKCMD(vkCreateSemaphore(vulkanDevice.vkDevice, &vkSemaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
        CHECK_VKCMD(vkCreateFence(vulkanDevice.vkDevice, &vkFenceInfo, nullptr, &queueSubmissionFences[i]));
    }
}

void VulkanCmdBuffer::DestroySynchPrimitives()
{
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;

    for (uint32_t i = 0; i < frame_in_flight; i++)
    {
        vkDestroySemaphore(vulkanDevice.vkDevice, imageAcquiredSemaphores[i], nullptr);
        vkDestroySemaphore(vulkanDevice.vkDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(vulkanDevice.vkDevice, queueSubmissionFences[i], nullptr);
    }
}

VkCommandBuffer VulkanCmdBuffer::SingleCmdBegin()
{
    if (singleCommandPool == VK_NULL_HANDLE) 
        Log::Write(Log::Level::Error, "[Vulkan CmdBuffer] Error, Command pool not initialized.");

    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;

    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = singleCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkanDevice.vkDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanCmdBuffer::SingleCmdEnd(VkCommandBuffer commandBuffer)
{
    VulkanDevice& vulkanDevice = VulkanRenderer::GetInstance().vulkanDevice;

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkanDevice.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanDevice.graphicsQueue);

    vkFreeCommandBuffers(vulkanDevice.vkDevice, singleCommandPool, 1, &commandBuffer);
}