#include "openxr_swapchain.h"

#include "openxr_platform.h"
#include "logger.h"
#include "validation.h"


static XrResult result;
static char resultBuffer[XR_MAX_STRUCTURE_NAME_SIZE];

#define CHK_XRCMD(result)                                                      \
    if (XR_FAILED(result)) {PrintErrorMsg(result);}     

#define IMAGE_FORMAT VK_FORMAT_R8G8B8A8_SRGB

void OpenxrSession::CreateSession(VulkanDevice* vulkanDevice)
{
    XrGraphicsBindingVulkanKHR vkBinding{XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR};
    vkBinding.instance = vulkanDevice->vkInstance;
    vkBinding.physicalDevice = vulkanDevice->vkPhysicalDevice;
    vkBinding.device = vulkanDevice->vkDevice;
    vkBinding.queueFamilyIndex = vulkanDevice->graphicsIndex;
    vkBinding.queueIndex = 0;

    PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR;
    CHK_XRCMD(xrGetInstanceProcAddr(
        platform->xrInstance, "xrGetVulkanGraphicsDeviceKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanGraphicsDeviceKHR)));

    // Ensure the physical device used by Vulkan renderer has a VR headset connected.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    xrGetVulkanGraphicsDeviceKHR(platform->xrInstance, platform->xrSystemId,
        vulkanDevice->vkInstance, &physicalDevice);
    if (physicalDevice != vulkanDevice->vkPhysicalDevice)
    {
        //TODO:
        throw;
    }

    //FIXME:  vkCreateShaderModule() vulkan validation error
    XrSessionCreateInfo createInfo{XR_TYPE_SESSION_CREATE_INFO};
    createInfo.next = &vkBinding;
    createInfo.systemId = platform->xrSystemId;
    CHK_XRCMD(xrCreateSession(platform->xrInstance, &createInfo, &xrSession));

    sessionState = XR_SESSION_STATE_IDLE;
}

void OpenxrSession::SetSessionState(XrSessionState newState)
{
    Logger::Write(
        "[OpenXR] Session state changes from " +
        std::to_string(sessionState) + " to " +
        std::to_string(newState),
        Logger::Level::Info, Logger::Platform
    );

    sessionState = newState;

    switch (sessionState)
    {
        case XR_SESSION_STATE_READY:
        {
            XrSessionBeginInfo info{XR_TYPE_SESSION_BEGIN_INFO};
            info.primaryViewConfigurationType = 
                XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
            CHK_XRCMD(xrBeginSession(xrSession, &info));
            break;
        }
        case XR_SESSION_STATE_STOPPING:
        {
            CHK_XRCMD(xrEndSession(xrSession))
            break;
        }
        case XR_SESSION_STATE_EXITING:
        {
            break;
        }
        case XR_SESSION_STATE_LOSS_PENDING:
        {
            break;
        }
        default:
            break;
    }

}

void OpenxrSession::InitializeSpaces()
{
    uint32_t spaceCount;
    CHK_XRCMD(xrEnumerateReferenceSpaces(xrSession, 0, &spaceCount, nullptr));
    std::vector<XrReferenceSpaceType> spaces(spaceCount);
    CHK_XRCMD(xrEnumerateReferenceSpaces(xrSession, spaceCount, &spaceCount, spaces.data()));

    Logger::Write(
        "[OpenXR] Available reference spaces: " + std::to_string(spaceCount),
        Logger::Level::Info, Logger::MsgType::Platform
    );

    for (XrReferenceSpaceType space: spaces)
    {
        std::string spaceName;
        switch (space)
        {
        case XR_REFERENCE_SPACE_TYPE_VIEW:
            spaceName = "XR_REFERENCE_SPACE_TYPE_VIEW";
            break;
        case XR_REFERENCE_SPACE_TYPE_LOCAL:
            spaceName = "XR_REFERENCE_SPACE_TYPE_LOCAL";
            break;
        case XR_REFERENCE_SPACE_TYPE_STAGE:
            spaceName = "XR_REFERENCE_SPACE_TYPE_STAGE";
            break;
        default:
            spaceName = "Unknown reference space: " + std::to_string(space);
            break;
        }
        Logger::Write("[OpenXR] Referenece space: " + spaceName,
            Logger::Level::Info, Logger::MsgType::Platform);
    }
    
    if (spaceCount != 3){
        Logger::Write(
            "[OpenXR] Reference space check failed.",
            Logger::Level::Error, Logger::MsgType::Platform
        );
    }

    {   // Create reference spaces
        XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
        createInfo.poseInReferenceSpace.orientation.w = 1.0f;
        createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
        CHK_XRCMD(xrCreateReferenceSpace(xrSession, &createInfo, &viewSpace));
        createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
        CHK_XRCMD(xrCreateReferenceSpace(xrSession, &createInfo, &localSpace));
        createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
        CHK_XRCMD(xrCreateReferenceSpace(xrSession, &createInfo, &stageSpace));
    }

    {   // Create action spaces
        XrActionSpaceCreateInfo createInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
        createInfo.poseInActionSpace.orientation.w = 1.0f;
        createInfo.action = platform->lGripPoseAction;
        CHK_XRCMD(xrCreateActionSpace(xrSession, &createInfo, &lGripPoseSpace));
        createInfo.action = platform->rGripPoseAction;
        CHK_XRCMD(xrCreateActionSpace(xrSession, &createInfo, &rGripPoseSpace));
        createInfo.action = platform->lAimPoseAction;
        CHK_XRCMD(xrCreateActionSpace(xrSession, &createInfo, &lAimPoseSpace));
        createInfo.action = platform->rAimPoseAction;
        CHK_XRCMD(xrCreateActionSpace(xrSession, &createInfo, &rAimPoseSpace));
    }
}

bool OpenxrSession::SelectImageFormat(VkFormat format)
{
    unsigned int count;
    std::vector<int64_t> formatList;
    xrEnumerateSwapchainFormats(xrSession, 0, &count, nullptr);
    formatList.resize(count);
    xrEnumerateSwapchainFormats(xrSession, count, &count, formatList.data());

    for(int64_t& xrFormat: formatList)
    {
        if (xrFormat == format)
        {
            this->imageFormat = xrFormat;
            return true;
        }
    }
    return false;
}

void OpenxrSession::Initialize(VulkanDevice* vulkanDevice)
{
    CreateSession(vulkanDevice);
    InitializeSpaces();

    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &platform->inputActionSet;
    CHK_XRCMD(xrAttachSessionActionSets(xrSession, &attachInfo));

    if (!SelectImageFormat(IMAGE_FORMAT))
    {
        Logger::Write(
            "[OpenXR] Unable to Get image format",
            Logger::Level::Error, Logger::MsgType::Platform
        );
    }

    // Create the swapchain for each eye
    std::vector<XrViewConfigurationView>& viewList = platform->viewConfigViewList;
    imageWidth = viewList[0].recommendedImageRectWidth;
    imageHeight = viewList[0].recommendedImageRectHeight;
    ASSERT(viewList.size() == 2);
    for (unsigned int i = 0; i < viewList.size(); i++)
    {
        XrSwapchainCreateInfo createInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        createInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT |
                                XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
        createInfo.format = imageFormat;
        createInfo.sampleCount = viewList[i].recommendedSwapchainSampleCount;
        createInfo.width = viewList[i].recommendedImageRectWidth;
        createInfo.height = viewList[i].recommendedImageRectHeight;
        createInfo.faceCount = 1;
        createInfo.arraySize = 1;
        createInfo.mipCount = 1;

        XrSwapchain xrSwapchain;
        CHK_XRCMD(xrCreateSwapchain(xrSession, &createInfo, &xrSwapchain));
        swapchainList.push_back(xrSwapchain);
    }

    uint32_t count;
    std::vector<XrSwapchainImageVulkanKHR> swapchainImages;
    xrEnumerateSwapchainImages(swapchainList[0], 0, &count, nullptr);
    imageCount = 2 * count; // total number of images.

    // Images from left eye (index 0)
    swapchainImages.resize(count, {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR});
    xrEnumerateSwapchainImages(swapchainList[0], count, &count, 
        reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImages.data()));
    for (XrSwapchainImageVulkanKHR& header: swapchainImages)
    {
        images.push_back(header.image);
    }

    xrEnumerateSwapchainImages(swapchainList[1], 0, &count, nullptr);
    ASSERT(count * 2 == imageCount); // Both have the same number of images.

    // Images from the right eye (index 1)
    swapchainImages.resize(count, {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR});
    xrEnumerateSwapchainImages(swapchainList[1], count, &count, 
        reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImages.data()));
    for (XrSwapchainImageVulkanKHR& header: swapchainImages)
    {
        images.push_back(header.image);
    }

    // Create image views
    VkImageViewCreateInfo vkImageViewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.format = static_cast<VkFormat>(imageFormat);
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vkImageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    imageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        vkImageViewCreateInfo.image = images[i];
        CHECK_VKCMD(vkCreateImageView(vulkanDevice->vkDevice,
            &vkImageViewCreateInfo, nullptr, &imageViews[i]));
    }

    framebuffers.resize(imageCount);

    Logger::Write(
        "[Openxr] Swapchains created with total image count: " +
        std::to_string(imageCount),
        Logger::Level::Info, Logger::MsgType::Platform
    );
}

void OpenxrSession::Destroy(VulkanDevice* vulkanDevice)
{
    xrDestroySpace(viewSpace);
    xrDestroySpace(localSpace);
    xrDestroySpace(stageSpace);

    xrDestroySpace(lGripPoseSpace);
    xrDestroySpace(rGripPoseSpace);
    xrDestroySpace(lAimPoseSpace);
    xrDestroySpace(rAimPoseSpace);

    for (size_t i = 0; i < framebuffers.size(); i++)
        if (framebuffers[i])
            vkDestroyFramebuffer(vulkanDevice->vkDevice, framebuffers[i], nullptr);

    for (size_t i = 0; i < imageViews.size(); i++) 
        vkDestroyImageView(vulkanDevice->vkDevice, imageViews[i], nullptr);
    
    for (size_t i = 0; i < swapchainList.size(); i++)
        xrDestroySwapchain(swapchainList[i]);
    
    xrDestroySession(xrSession);
}

uint32_t OpenxrSession::GetNextImageIndex(VulkanDevice* vulkanDevice,
    VkSemaphore imageAcquiredSemaphores)
{
    return 0;
}

void OpenxrSession::PresentImage(VulkanDevice* vulkanDevic,
    VkSemaphore renderFinishedSemaphores, uint32_t imageIndex)
{

}


void OpenxrSession::RebuildSwapchain(VulkanDevice* vulkanDevice)
{
    // No resize window.
}

// Hardcoded to be VK_FORMAT_R8G8B8A8_SRGB
VkFormat OpenxrSession::GetImageFormat()
{
    return IMAGE_FORMAT;
}

uint32_t OpenxrSession::GetImageCount()
{
    return imageCount;
}
uint32_t OpenxrSession::GetWidth()
{
    return imageWidth;
}

uint32_t OpenxrSession::GetHeight()
{
    return imageHeight;
}

VkImage OpenxrSession::GetImage(int index)
{
    return images[index];
}

VkImageView OpenxrSession::GetImageView(int index)
{
    return imageViews[index];
}

VkFramebuffer* OpenxrSession::GetFramebuffer(int index)
{
    return &framebuffers[index];
}

void OpenxrSession::PrintErrorMsg(XrResult result)
{
    xrResultToString(platform->xrInstance, result, resultBuffer);
    Logger::Write(
        "[OpenXR] API call error: " + std::string(resultBuffer),
        Logger::Level::Error, Logger::MsgType::Platform
    );
}

//TODO: query interaction profile
    // XrPath leftSubpath, rightSubpath;
    // XrInteractionProfileState profileState{XR_TYPE_INTERACTION_PROFILE_STATE};
    // CHK_XRCMD(xrStringToPath(xrInstance, "/user", &leftSubpath));
    // CHK_XRCMD(xrStringToPath(xrInstance, "/user", &rightSubpath));
    // CHK_XRCMD(xrGetCurrentInteractionProfile(xrSession, leftSubpath, &profileState));

    // unsigned int count;
    // XrPath profilePath = profileState.interactionProfile;
    // std::vector<char> profileStr;
    // CHK_XRCMD(xrPathToString(xrInstance, profilePath, 0, &count, nullptr));
    // profileStr.resize(count);
    // CHK_XRCMD(xrPathToString(
    //     xrInstance, profilePath, count, &count, profileStr.data()));

    // Logger::Write(
    //     "[OpenXR] Current profile: " + std::string(profileStr.data()),
    //     Logger::Level::Info,
    //     Logger::MsgType::Platform
    // );