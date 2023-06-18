#pragma once

#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "vulkan_swapchain.h"
#include "vk_primitives/vulkan_device.h"
#include "input.h"

#include <vector>

class OpenxrSession;

class XrPlatform
{
public:
    virtual bool SystemFound() = 0;
    virtual OpenxrSession* NewSession() = 0;
    virtual bool ShouldCloseSession() = 0;
    virtual void RequestCloseSession() = 0;
    virtual bool IsSessionRunning() = 0;
    virtual void Destroy() = 0;

    virtual void PollEvents() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    virtual IVulkanSwapchain* GetSwapchain() = 0;
    virtual std::vector<const char*> GetVkInstanceExt() = 0;
    virtual std::vector<const char*> GetVkDeviceExt() = 0;

    virtual ~XrPlatform() = default;
};

class XrPlatformNotFound: public XrPlatform
{
public:
    bool SystemFound() override {return false;}
    OpenxrSession* NewSession() override {return nullptr;}
    bool ShouldCloseSession() override {return false;}
    void RequestCloseSession() override {};
    bool IsSessionRunning() override {return false;};
    void Destroy() override {};

    void PollEvents() override {}
    void BeginFrame() override {}
    void EndFrame() override {}

    IVulkanSwapchain* GetSwapchain() override {return nullptr;}
    std::vector<const char*> GetVkInstanceExt() override 
        {return std::vector<const char*>();};
    std::vector<const char*> GetVkDeviceExt() override
        {return std::vector<const char*>();}

    ~XrPlatformNotFound() override = default;
};

class OpenxrPlatform: public XrPlatform
{
public:
    /**
     * @brief Get an instance of the OpenXR platform.
     * If openxr instance cannot be created, an empty platform is returned.
     * @return OpenxrPlatform* 
     */
    static XrPlatform* Initialize();

    bool SystemFound() override {return true;}
    OpenxrSession* NewSession() override;
    bool ShouldCloseSession() override;
    void RequestCloseSession() override;
    bool IsSessionRunning() override;
    void Destroy() override;

    void PollEvents() override;
    void BeginFrame() override;
    void EndFrame() override;

    IVulkanSwapchain* GetSwapchain() override {return nullptr;};
    std::vector<const char*> GetVkInstanceExt() override {return vulkanInstanceExt;}
    std::vector<const char*> GetVkDeviceExt() override {return vulkanDeviceExt;}

    const OpenxrPlatform& operator=(const OpenxrPlatform&) = delete;
    OpenxrPlatform(const OpenxrPlatform&) = delete;

    ~OpenxrPlatform() override = default;

    friend OpenxrSession;

private:
    OpenxrPlatform() = default;

    bool TryReadNextEvent(XrEventDataBuffer* eventDataBuffer);

    void InitializeActions();
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
    XrSystemId xrSystemId = XR_NULL_SYSTEM_ID;
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

    //Running session
    OpenxrSession* session = nullptr;
};