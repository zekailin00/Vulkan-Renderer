#pragma once 

#include "vulkan_swapchain.h"
#include "vk_primitives/vulkan_device.h"

#include <vulkan/vulkan.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <vector>


class OpenxrPlatform;
class OpenxrSession: public IVulkanSwapchain
{

public:
    void SetOpenxrContext(OpenxrPlatform* platform)
    {
        this->platform = platform;
    }

    void SetSessionState(XrSessionState newState);

    void BeginFrame();
    void EndFrame();

    XrSessionState GetSessionState() {return sessionState;}
    XrSession GetSession() {return xrSession;}

    // XrSpace GetViewSpace() {return viewSpace;}
    // XrSpace GetLocalSpace() {return localSpace;}
    // XrSpace GetStageSpace() {return stageSpace;}

    // XrSpace GetLGripPoseSpace() {return lGripPoseSpace;}
    // XrSpace GetRGripPoseSpace() {return rGripPoseSpace;}
    // XrSpace GetLAimPoseSpace() {return lAimPoseSpace;}
    // XrSpace GetRAimPoseSpace() {return rAimPoseSpace;}

public:
    // Swapchain interface
    VkFormat GetImageFormat() override;
    uint32_t GetImageCount() override;
    uint32_t GetWidth() override;
    uint32_t GetHeight() override;

    VkImage GetImage(int index) override;
    VkImageView GetImageView(int index) override;
    VkFramebuffer* GetFramebuffer(int index) override;

    void Initialize(VulkanDevice* vulkanDevice) override;
    void Destroy(VulkanDevice* vulkanDevice) override;
    uint32_t GetNextImageIndex(VulkanDevice* vulkanDevice,
        VkSemaphore imageAcquiredSemaphores) override;
    void PresentImage(VulkanDevice* vulkanDevice,
        VkSemaphore renderFinishedSemaphores, uint32_t imageIndex) override;
    bool ShouldRender() override;

    void RebuildSwapchain(VulkanDevice* vulkanDevice) override;

private:
    // Helper methods
    void CreateSession(VulkanDevice* vukanDevice);
    void InitializeSpaces();
    bool SelectImageFormat(VkFormat format);
    void PrintErrorMsg(XrResult result);

private:    
    OpenxrPlatform* platform = nullptr;


    XrSession xrSession = XR_NULL_HANDLE;
    std::vector<XrSwapchain> swapchainList{};
    std::vector<VkImage> images{};
    std::vector<VkImageView> imageViews{};
    std::vector<VkFramebuffer> framebuffers{};

    XrSessionState sessionState = XR_SESSION_STATE_UNKNOWN;

    int64_t imageFormat = 0;
    uint32_t totalImageCount = 0; //Total images (per eye * 2)
    uint32_t imageCount = 0;
    uint32_t imageWidth = 0;
    uint32_t imageHeight = 0;


    XrSpace viewSpace = VK_NULL_HANDLE;
    XrSpace localSpace = VK_NULL_HANDLE;
    XrSpace stageSpace = VK_NULL_HANDLE;

    XrSpace lGripPoseSpace = VK_NULL_HANDLE;
    XrSpace rGripPoseSpace = VK_NULL_HANDLE;
    XrSpace lAimPoseSpace = VK_NULL_HANDLE;
    XrSpace rAimPoseSpace = VK_NULL_HANDLE;


    // Render loop context
    // reset when enter XR_SESSION_STATE_READY
    // index 0 = left eye, index 1 = right eye
    XrFrameState frameState{};
    XrCompositionLayerProjection layer;
    XrCompositionLayerProjectionView layerViews[2]; 
    XrView views[2];
    int eye;
};