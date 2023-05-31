#pragma once

#include "vulkan/vulkan.h"

#include "vulkan_swapchain.h"

#include <vector>


class WindowSwapchain: public IVulkanSwapchain
{
public:
    virtual uint32_t GetImageCount() override {return imageCount;}
    virtual VkFormat GetImageFormat() override {return vkFormat;}
    virtual uint32_t GetWidth() override {return vkExtent.width;}
    virtual uint32_t GetHeight() override {return vkExtent.height;}

    virtual VkImage GetImage(int index) override {return images[index];}
    virtual VkImageView GetImageView(int index) override {return imageViews[index];}

    virtual uint32_t GetNextImageIndex(VkSemaphore imageAcquiredSemaphores) override;
    virtual void PresentImage(VkSemaphore renderFinishedSemaphores, uint32_t imageIndex) override;
    virtual void RebuildSwapchain() override;
    virtual void Initialize() override;
    virtual void Destroy() override;

    void GetSwapChainProperties();
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