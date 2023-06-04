#pragma once 

#include "vk_primitives/vulkan_device.h"
#include <vulkan/vulkan.h>


class IVulkanSwapchain
{
public:
    virtual VkFormat GetImageFormat() = 0;
    virtual uint32_t GetImageCount() = 0;
    virtual uint32_t GetWidth() = 0;
    virtual uint32_t GetHeight() = 0;

    virtual VkImage GetImage(int index) = 0;
    virtual VkImageView GetImageView(int index) = 0;
    virtual VkFramebuffer* GetFramebuffer(int index) = 0;

    virtual void Initialize(VulkanDevice* vulkanDevice) = 0;
    virtual void Destroy(VulkanDevice* vulkanDevice) = 0;
    virtual uint32_t GetNextImageIndex(VulkanDevice* vulkanDevice,
        VkSemaphore imageAcquiredSemaphores) = 0;
    virtual void PresentImage(VulkanDevice* vulkanDevic,
        VkSemaphore renderFinishedSemaphores, uint32_t imageIndex) = 0;
    virtual bool ShouldRender() = 0;

    /**
     * @brief Rebuild swapchain, images, and image views. 
     * It should be called internally by the renderer, not by the window system that implements the interface.
     * 
     */
    virtual void RebuildSwapchain(VulkanDevice* vulkanDevice) = 0;
};