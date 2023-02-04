#pragma once 

#include "vulkan/vulkan.h"

class IVulkanSwapchain
{
public:
    virtual VkFormat GetImageFormat() = 0;
    virtual uint32_t GetImageCount() = 0;
    virtual uint32_t GetWidth() = 0;
    virtual uint32_t GetHeight() = 0;

    virtual VkImage GetImage(int index) = 0;
    virtual VkImageView GetImageView(int index) = 0;

    virtual void Initialize() = 0;
    virtual void Destroy() = 0;
    virtual uint32_t GetNextImageIndex(VkSemaphore imageAcquiredSemaphores) = 0;
    virtual void PresentImage(VkSemaphore renderFinishedSemaphores, uint32_t imageIndex) = 0;

    /**
     * @brief Rebuild swapchain, images, and image views. 
     * It should be called internally by the renderer, not by the window system that implements the interface.
     * 
     */
    virtual void RebuildSwapchain() = 0;
};