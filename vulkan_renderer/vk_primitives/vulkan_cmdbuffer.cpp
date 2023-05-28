#include "vulkan_cmdbuffer.h"

#include "validation.h"
#include "logger.h"

#include <vector>
#include <tracy/Tracy.hpp>


void VulkanCmdBuffer::Initialize(VulkanDevice* vulkanDevice, uint32_t frameInFlight)
{
    ZoneScopedN("VulkanCmdBuffer::Initialize");

    this->frameInFlight = frameInFlight;
    this->vulkanDevice = vulkanDevice;

    VkCommandPoolCreateInfo vkCommandPoolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    vkCommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolInfo.queueFamilyIndex = vulkanDevice->graphicsIndex;
    CHECK_VKCMD(vkCreateCommandPool(vulkanDevice->vkDevice, &vkCommandPoolInfo, nullptr, &vkCommandPool));

    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = frameInFlight;
    vkCommandBuffers.resize(frameInFlight);
    CHECK_VKCMD(vkAllocateCommandBuffers(vulkanDevice->vkDevice, &vkCommandBufferAllocateInfo, vkCommandBuffers.data()));

    CreateSynchPrimitives();
}

void VulkanCmdBuffer::Destroy()
{
    ZoneScopedN("VulkanCmdBuffer::Destroy");

    if (vulkanDevice == nullptr)
        return;

    vkDestroyCommandPool(vulkanDevice->vkDevice, vkCommandPool, nullptr);
    DestroySynchPrimitives();
    vulkanDevice = nullptr;
    frameInFlight = 0;
    currentFrame = 0;
}

void VulkanCmdBuffer::CreateSynchPrimitives()
{
    ZoneScopedN("VulkanCmdBuffer::CreateSynchPrimitives");

    VkSemaphoreCreateInfo vkSemaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo vkFenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    vkFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    imageAcquiredSemaphores.resize(frameInFlight);
    renderFinishedSemaphores.resize(frameInFlight);
    queueSubmissionFences.resize(frameInFlight);
    for (uint32_t i = 0; i < frameInFlight; i++)
    {
        CHECK_VKCMD(vkCreateSemaphore(vulkanDevice->vkDevice, &vkSemaphoreInfo, nullptr, &imageAcquiredSemaphores[i]));
        CHECK_VKCMD(vkCreateSemaphore(vulkanDevice->vkDevice, &vkSemaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
        CHECK_VKCMD(vkCreateFence(vulkanDevice->vkDevice, &vkFenceInfo, nullptr, &queueSubmissionFences[i]));
    }
}

void VulkanCmdBuffer::DestroySynchPrimitives()
{
    ZoneScopedN("VulkanCmdBuffer::DestroySynchPrimitives");

    for (uint32_t i = 0; i < frameInFlight; i++)
    {
        vkDestroySemaphore(vulkanDevice->vkDevice, imageAcquiredSemaphores[i], nullptr);
        vkDestroySemaphore(vulkanDevice->vkDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(vulkanDevice->vkDevice, queueSubmissionFences[i], nullptr);
    }
}

VkCommandBuffer VulkanCmdBuffer::BeginCommand()
{
    ZoneScopedN("VulkanCmdBuffer::BeginCommand");

    CHECK_VKCMD(vkResetFences(
        vulkanDevice->vkDevice, 1, &queueSubmissionFences[currentFrame]));
    CHECK_VKCMD(vkResetCommandBuffer(
        vkCommandBuffers[currentFrame], 0));

    VkCommandBufferBeginInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CHECK_VKCMD(vkBeginCommandBuffer(vkCommandBuffers[currentFrame], &info));

    return vkCommandBuffers[currentFrame];
}

void VulkanCmdBuffer::EndCommand()
{
    ZoneScopedN("VulkanCmdBuffer::EndCommand");

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo vkSubmitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &imageAcquiredSemaphores[currentFrame];
    vkSubmitInfo.pWaitDstStageMask = &waitStage;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffers[currentFrame];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

    CHECK_VKCMD(vkEndCommandBuffer(vkCommandBuffers[currentFrame]));
    CHECK_VKCMD(vkQueueSubmit(vulkanDevice->graphicsQueue, 1, &vkSubmitInfo, queueSubmissionFences[currentFrame]));
    CHECK_VKCMD(vkWaitForFences(
        vulkanDevice->vkDevice, 1, &queueSubmissionFences[currentFrame], VK_TRUE, UINT64_MAX));
    currentFrame = (currentFrame + 1) % frameInFlight;
}

VkCommandBuffer VulkanSingleCmd::BeginCommand()
{
    ZoneScopedN("VulkanSingleCmd::BeginCommand");

    if (vkCommandBuffer == VK_NULL_HANDLE) 
        Log::Write(Log::Level::Error, 
            "[Vulkan CmdBuffer] Error, Command buffer not initialized.");

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);
    CHECK_VKCMD(vkResetFences(vulkanDevice->vkDevice, 1, &vkFence));
    return vkCommandBuffer;
}

void VulkanSingleCmd::EndCommand()
{
    ZoneScopedN("VulkanSingleCmd::EndCommand");

    if (vkCommandBuffer == VK_NULL_HANDLE) 
        Log::Write(Log::Level::Error, 
            "[Vulkan CmdBuffer] Error, Command buffer not initialized.");
    
    vkEndCommandBuffer(vkCommandBuffer);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCommandBuffer;

    CHECK_VKCMD(vkQueueSubmit(vulkanDevice->graphicsQueue, 1, &submitInfo, vkFence));
    CHECK_VKCMD(vkWaitForFences(vulkanDevice->vkDevice, 1, &vkFence, VK_TRUE, UINT64_MAX));
    CHECK_VKCMD(vkResetFences(vulkanDevice->vkDevice, 1, &vkFence));
}

void VulkanSingleCmd::Initialize(VulkanDevice* vulkanDevice)
{
    ZoneScopedN("VulkanSingleCmd::Initialize");

    this->vulkanDevice = vulkanDevice;

    VkCommandPoolCreateInfo vkCommandPoolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    vkCommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolInfo.queueFamilyIndex = vulkanDevice->graphicsIndex;
    CHECK_VKCMD(vkCreateCommandPool(vulkanDevice->vkDevice, &vkCommandPoolInfo, nullptr, &vkCommandPool));

    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vkCommandPool;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(vulkanDevice->vkDevice, &allocInfo, &vkCommandBuffer);

    VkFenceCreateInfo vkFenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    vkFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    CHECK_VKCMD(vkCreateFence(vulkanDevice->vkDevice, &vkFenceInfo, nullptr, &vkFence));
}

void VulkanSingleCmd::Destroy()
{
    ZoneScopedN(" VulkanSingleCmd::Destroy");

    if (vulkanDevice == nullptr)
        return;

    vkDestroyFence(vulkanDevice->vkDevice, vkFence, nullptr);
    vkFreeCommandBuffers(vulkanDevice->vkDevice, vkCommandPool, 1, &vkCommandBuffer);
    vkDestroyCommandPool(vulkanDevice->vkDevice, vkCommandPool, nullptr);

    vkFence = VK_NULL_HANDLE;
    vkCommandPool = VK_NULL_HANDLE;
    vkCommandBuffer = VK_NULL_HANDLE;

    vulkanDevice = nullptr;
}