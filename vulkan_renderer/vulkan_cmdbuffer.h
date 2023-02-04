#pragma once 

#include "vulkan/vulkan.h"

#include <vector>

class VulkanCmdBuffer
{
public:
    void Initialize(uint32_t frame_in_flight);
    void Destroy();

    VkCommandBuffer& GetCurrCmdBuffer() {return vkCommandBuffers[currentFrame];}
    VkSemaphore& GetCurrImageSemaphore() {return imageAcquiredSemaphores[currentFrame];}
    VkSemaphore& GetCurrRenderSemaphore() {return renderFinishedSemaphores[currentFrame];}
    VkFence& GetCurrSubmissionFence() {return queueSubmissionFences[currentFrame];}
    uint32_t NextFrame() {return currentFrame = (currentFrame + 1) % frame_in_flight;}
    uint32_t CurrentFrame() {return currentFrame;}

    VkCommandBuffer SingleCmdBegin();
    void SingleCmdEnd(VkCommandBuffer commandBuffer);

    uint32_t frame_in_flight;
    uint32_t currentFrame = 0;

    std::vector<VkCommandBuffer> vkCommandBuffers;
    std::vector<VkSemaphore> imageAcquiredSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> queueSubmissionFences;

    VulkanCmdBuffer() = default;
    VulkanCmdBuffer(VulkanCmdBuffer const&) = delete;
    void operator=(VulkanCmdBuffer const&) = delete;

private:
    void CreateSynchPrimitives();
    void DestroySynchPrimitives();

    VkCommandPool vkCommandPool = VK_NULL_HANDLE;
    VkCommandPool singleCommandPool = VK_NULL_HANDLE;
};