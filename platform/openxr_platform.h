#pragma once

#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <vector>

#include "vulkan_swapchain.h"


class OpenxrPlatform
{
public:
    /**
     * @brief Get an instance of the OpenXR platform.
     * If openxr instance cannot be created, nullptr is returned.
     * @return OpenxrPlatform* 
     */
    static OpenxrPlatform* Initialize();

    IVulkanSwapchain* GetSwapchain() {return nullptr;};
    std::vector<const char*> GetVkInstanceExt() {return vulkanInstanceExt;}
    std::vector<const char*> GetVkDeviceExt() {return vulkanDeviceExt;}

    const OpenxrPlatform& operator=(const OpenxrPlatform&) = delete;
    OpenxrPlatform(const OpenxrPlatform&) = delete;

    ~OpenxrPlatform() = default;

private:
    OpenxrPlatform() = default;

    void LoadViewConfig();
    void LoadVulkanRequirements();
    void PrintErrorMsg(XrResult result);
    std::vector<const char*> ParseExtensionString(char* names);

private:
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
    XrSystemId systemId = XR_NULL_HANDLE;
};