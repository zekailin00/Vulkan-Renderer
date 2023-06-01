#pragma once

#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "vulkan_swapchain.h"
#include "vk_primitives/vulkan_device.h"

#include <vector>


class OpenxrPlatform
{
public:
    /**
     * @brief Get an instance of the OpenXR platform.
     * If openxr instance cannot be created, nullptr is returned.
     * @return OpenxrPlatform* 
     */
    static OpenxrPlatform* Initialize();

    void InitializeSession(VulkanDevice* vulkanDevice);

    IVulkanSwapchain* GetSwapchain() {return nullptr;};
    std::vector<const char*> GetVkInstanceExt() {return vulkanInstanceExt;}
    std::vector<const char*> GetVkDeviceExt() {return vulkanDeviceExt;}

    const OpenxrPlatform& operator=(const OpenxrPlatform&) = delete;
    OpenxrPlatform(const OpenxrPlatform&) = delete;

    ~OpenxrPlatform() = default;

private:
    OpenxrPlatform() = default;

    void InitializeActions();
    void InitializeSpaces();

    void LoadViewConfig();
    void LoadVulkanRequirements();
    void PrintErrorMsg(XrResult result);
    std::vector<const char*> ParseExtensionString(char* names);

private:
    //------- Instance data -------//
    std::vector<XrApiLayerProperties> layerList{};
    std::vector<XrExtensionProperties> extensionList{};
    std::vector<XrViewConfigurationType> viewConfigTypeList{};
    std::vector<XrViewConfigurationView> viewConfigViewList{};

    XrGraphicsRequirementsVulkanKHR vkRequirements{};
    std::vector<char> vulkanInstanceExtStr;
    std::vector<const char*> vulkanInstanceExt{};
    std::vector<char> vulkanDeviceExtStr;
    std::vector<const char*> vulkanDeviceExt{};

    XrInstance xrInstance = XR_NULL_HANDLE;
    XrSystemId systemId = XR_NULL_SYSTEM_ID;
    XrActionSet inputActionSet = XR_NULL_HANDLE;

    XrAction lSqueezeValueAction;
    XrAction rSqueezeValueAction;

    XrAction lTriggerValueAction;
    XrAction rTriggerValueAction;

    XrAction lTriggerTouchAction;
    XrAction rTriggerTouchAction;

    XrAction lThumbstickXAction;
    XrAction rThumbstickXAction;

    XrAction lThumbstickYAction;
    XrAction rThumbstickYAction;

    XrAction lThumbstickClickAction;
    XrAction rThumbstickClickAction;

    XrAction lThumbstickTouchAction;
    XrAction rThumbstickTouchAction;

    XrAction lXClickAction;
    XrAction lXTouchAction;
    XrAction lYClickAction;
    XrAction lYTouchAction;
    XrAction lMenuClickAction;

    XrAction rAClickAction;
    XrAction rATouchAction;
    XrAction rBClickAction;
    XrAction rBTouchAction;
    XrAction rSystemClickAction;

    XrAction lGripPoseAction;
    XrAction rGripPoseAction;
    XrAction lAimPoseAction;
    XrAction rAimPoseAction;

    //-------  Session data -------//
    XrSession xrSession = XR_NULL_HANDLE;
    XrSpace viewSpace;
    XrSpace localSpace;
    XrSpace stageSpace;
    XrSpace lGripPoseSpace;
    XrSpace rGripPoseSpace;
    XrSpace lAimPoseSpace;
    XrSpace rAimPoseSpace;

};