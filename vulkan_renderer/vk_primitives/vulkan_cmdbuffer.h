#pragma once 

#include <vulkan/vulkan.h>
#include "vk_primitives/vulkan_device.h"

#include <vector>


class VulkanCmdBuffer
{
public:
    void Initialize(VulkanDevice* vulkanDevice, uint32_t frameInFlight);
    void Destroy();

    VkCommandBuffer BeginCommand();
    void EndCommand();

    VkSemaphore GetCurrImageSemaphore() {return imageAcquiredSemaphores[currentFrame];}
    VkSemaphore GetCurrRenderSemaphore() {return renderFinishedSemaphores[currentFrame];}

    VulkanCmdBuffer() = default;
    ~VulkanCmdBuffer(){Destroy();}

    VulkanCmdBuffer(const VulkanCmdBuffer&) = delete;
    VulkanCmdBuffer& operator=(const VulkanCmdBuffer&) = delete;

private:
    void CreateSynchPrimitives();
    void DestroySynchPrimitives();

    VulkanDevice* vulkanDevice = nullptr;
    VkCommandPool vkCommandPool = VK_NULL_HANDLE;

    uint32_t frameInFlight = 0;
    uint32_t currentFrame = 0;

    std::vector<VkCommandBuffer> vkCommandBuffers;
    std::vector<VkSemaphore> imageAcquiredSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> queueSubmissionFences;
};

class VulkanSingleCmd
{
public:
    void Initialize(VulkanDevice* vulkanDevice);
    void Destroy();

    VkCommandBuffer BeginCommand();
    void EndCommand();

    VulkanSingleCmd() = default;
    ~VulkanSingleCmd(){Destroy();}

    VulkanSingleCmd(const VulkanSingleCmd&) = delete;
    VulkanSingleCmd& operator=(const VulkanSingleCmd&) = delete;

private:
    VulkanDevice* vulkanDevice = nullptr;
    VkFence vkFence = VK_NULL_HANDLE;
    VkCommandPool vkCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;
};