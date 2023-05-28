#include "window_swapchain.h"

#include "vulkan_renderer.h"
#include "validation.h"


void WindowSwapchain::Initialize()
{
    ZoneScopedN("WindowSwapchain::Initialize");

    renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();

    //TODO: select other available swapchain properties by the caller
    // Swapchain settings
    VkSurfaceFormatKHR surfaceFormat = vkSurfaceFormats[0];
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Always available
    VkExtent2D extent = vkExtent = vkSurfaceCapabilities.currentExtent;
    const uint32_t minImageCount = 3;

    // Create swapchain
    VkSwapchainCreateInfoKHR vkSwapchainInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    vkSwapchainInfo.surface = vkSurface;
    vkSwapchainInfo.minImageCount = minImageCount;
    vkSwapchainInfo.imageFormat = vkFormat = surfaceFormat.format;
    vkSwapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    vkSwapchainInfo.imageExtent = extent;
    vkSwapchainInfo.imageArrayLayers = 1; // Not stereo display
    vkSwapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkSwapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Assume graphicsQueue == presentQueue
    vkSwapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    vkSwapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainInfo.presentMode = presentMode;
    vkSwapchainInfo.clipped = VK_TRUE;
    vkSwapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    // Asserts
    CHECK_VKCMD(vkCreateSwapchainKHR(vkr.vulkanDevice.vkDevice, &vkSwapchainInfo, nullptr, &vkSwapchain));
    CHECK_VKCMD(vkGetSwapchainImagesKHR(vkr.vulkanDevice.vkDevice, vkSwapchain, &imageCount, nullptr));
    images.resize(imageCount);
    CHECK_VKCMD(vkGetSwapchainImagesKHR(vkr.vulkanDevice.vkDevice, vkSwapchain, &imageCount, images.data()));
    ASSERT(minImageCount <= imageCount)

    // Create image views
    VkImageViewCreateInfo vkImageViewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.format = surfaceFormat.format;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vkImageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    imageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        vkImageViewCreateInfo.image = images[i];
        CHECK_VKCMD(vkCreateImageView(vkr.vulkanDevice.vkDevice, &vkImageViewCreateInfo, nullptr, &imageViews[i]));
    }

    std::cout << "[GLFW Window] Swapchain created with image count: " << imageCount << std::endl;
}

void WindowSwapchain::Destroy()
{
    ZoneScopedN("WindowSwapchain::Destroy");

    renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();

    for (size_t i = 0; i < imageViews.size(); i++) 
        vkDestroyImageView(vkr.vulkanDevice.vkDevice, imageViews[i], nullptr);

    vkDestroySwapchainKHR(vkr.vulkanDevice.vkDevice, vkSwapchain, nullptr);
}

uint32_t WindowSwapchain::GetNextImageIndex(VkSemaphore imageAcquiredSemaphores)
{
    ZoneScopedN("WindowSwapchain::GetNextImageIndex");

    uint32_t imageIndex; 
    VkDevice vkd = renderer::VulkanRenderer::GetInstance().vulkanDevice.vkDevice;
    VkResult result = vkAcquireNextImageKHR(vkd, vkSwapchain, UINT64_MAX, imageAcquiredSemaphores, VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        swapchainRebuild = true;
    return imageIndex;
};

void WindowSwapchain::PresentImage(VkSemaphore renderFinishedSemaphores, uint32_t imageIndex)
{
    ZoneScopedN("WindowSwapchain::PresentImage");

    if (swapchainRebuild)
        return;
    VkPresentInfoKHR vkPresentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    vkPresentInfo.waitSemaphoreCount = 1;
    vkPresentInfo.pWaitSemaphores = &renderFinishedSemaphores;
    vkPresentInfo.swapchainCount = 1;
    vkPresentInfo.pSwapchains = &vkSwapchain;
    vkPresentInfo.pImageIndices = &imageIndex;
    VkResult result = vkQueuePresentKHR(renderer::VulkanRenderer::GetInstance().vulkanDevice.graphicsQueue, &vkPresentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        swapchainRebuild = true;
}

void WindowSwapchain::RebuildSwapchain()
{
    ZoneScopedN("WindowSwapchain::RebuildSwapchain");

    Destroy();
    GetSwapChainProperties(); // Get updated extent information 
    Initialize();
}

void WindowSwapchain::GetSwapChainProperties()
{
    ZoneScopedN("WindowSwapchain::GetSwapChainProperties");

    renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkr.vulkanDevice.vkPhysicalDevice, vkSurface, &vkSurfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkr.vulkanDevice.vkPhysicalDevice, vkSurface, &formatCount, nullptr);
    vkSurfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkr.vulkanDevice.vkPhysicalDevice, vkSurface, &formatCount, vkSurfaceFormats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkr.vulkanDevice.vkPhysicalDevice, vkSurface, &presentModeCount, nullptr);
    vkPresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkr.vulkanDevice.vkPhysicalDevice, vkSurface, &presentModeCount, vkPresentModes.data());

    if (vkSurfaceFormats.empty() || vkPresentModes.empty()) {
        std::cout << "[GLFW] Error: Surface properties not found" << std::endl;
        exit(1);
    }
}

void WindowSwapchain::SetSurface(VkSurfaceKHR vkSurface)
{
    ZoneScopedN("WindowSwapchain::SetSurface");

    this->vkSurface = vkSurface;
}