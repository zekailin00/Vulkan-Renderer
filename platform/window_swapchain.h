#pragma once

#include "vulkan/vulkan.h"

#include "vulkan_swapchain.h"

#include <vector>


/**
 * @brief Swapchain of GLFW window. It is owned by GLFW window,
 * but managed and used by the Vulkan renderer.
 * It's memory is freed when GLFW window is destroyed.
 * However, Vulkan renderer can reset (initialize and destroy) it at any time.
 */
class WindowSwapchain: public IVulkanSwapchain
{
public:
    virtual uint32_t GetImageCount() override {return imageCount;}
    virtual VkFormat GetImageFormat() override {return vkFormat;}
    virtual uint32_t GetWidth() override {return vkExtent.width;}
    virtual uint32_t GetHeight() override {return vkExtent.height;}

    virtual VkImage GetImage(int index) override {return images[index];}
    virtual VkImageView GetImageView(int index) override {return imageViews[index];}

    virtual uint32_t GetNextImageIndex(VulkanDevice* vulkanDevice,
        VkSemaphore imageAcquiredSemaphores) override;
    virtual void PresentImage(VulkanDevice* vulkanDevice,
        VkSemaphore renderFinishedSemaphores, uint32_t imageIndex) override;
    virtual void RebuildSwapchain(VulkanDevice* vulkanDevice) override;
    virtual void Initialize(VulkanDevice* vulkanDevice) override;
    virtual void Destroy(VulkanDevice* vulkanDevice) override;

    void GetSwapChainProperties(VulkanDevice* vulkanDevice);
    void SetSurface(VkSurfaceKHR vkSurface);

    bool swapchainRebuild = false;

private:
    VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;

    uint32_t imageCount;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    VkExtent2D vkExtent;
    VkFormat vkFormat;

    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> vkSurfaceFormats{};
    std::vector<VkPresentModeKHR> vkPresentModes{};
    VkSurfaceKHR vkSurface; // Acquired from window system
};