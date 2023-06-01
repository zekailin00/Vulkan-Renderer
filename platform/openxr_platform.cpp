#include "openxr_platform.h"

#include "logger.h"

#include <vector>
#include <string>

#define XR_SUCCEEDED(result) ((result) >= 0)
#define XR_FAILED(result) ((result) < 0)
#define XR_UNQUALIFIED_SUCCESS(result) ((result) == 0)

static XrResult result;
static char resultBuffer[XR_MAX_STRUCTURE_NAME_SIZE];

#define CHK_XRCMD(result)                                                      \
    if (XR_FAILED(result)) {PrintErrorMsg(result);}                                                                   

OpenxrPlatform* OpenxrPlatform::Initialize()
{
    unsigned int layerCount;
    std::vector<XrApiLayerProperties> layerList;
    unsigned int extensionCount;
    std::vector<XrExtensionProperties> extensionList;

    xrEnumerateApiLayerProperties(0, &layerCount, nullptr);
    layerList.resize(layerCount, {XR_TYPE_API_LAYER_PROPERTIES});
    xrEnumerateApiLayerProperties(layerCount, &layerCount, layerList.data());

    Logger::Write(
        "[OpenXR] Layer count: " + std::to_string(layerCount),
        Logger::Level::Info, Logger::MsgType::Platform 
    );

    for (auto& layer: layerList)
    {
        Logger::Write(
            layer.layerName,
            Logger::Level::Verbose, Logger::MsgType::Platform
        );
    }

    xrEnumerateInstanceExtensionProperties(
        nullptr, 0, &extensionCount, nullptr
    );
    extensionList.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    xrEnumerateInstanceExtensionProperties(
        nullptr, extensionCount, &extensionCount, extensionList.data()
    );

    Logger::Write(
        "[OpenXR] Extension count: " + std::to_string(extensionCount),
        Logger::Level::Info, Logger::MsgType::Platform
    );

    for (auto& extension: extensionList)
    {
        Logger::Write(
            extension.extensionName,
            Logger::Level::Verbose, Logger::MsgType::Platform
        );
    }

    XrInstanceCreateInfo instanceInfo{XR_TYPE_INSTANCE_CREATE_INFO};
    instanceInfo.applicationInfo = {
        "SemaLink-Platform", 1, "SemaLink-Engine", 1,
        XR_CURRENT_API_VERSION
    };

    std::vector<char*> enabledExt = {
        "XR_KHR_vulkan_enable", "XR_EXT_debug_utils"
    };
    instanceInfo.enabledExtensionCount = enabledExt.size();
    instanceInfo.enabledExtensionNames = enabledExt.data();

    XrInstance xrInstance;
    result = xrCreateInstance(&instanceInfo, &xrInstance);

    if (XR_FAILED(result)) {
        Logger::Write(
            "[OpenXR] Failed to load OpenXR runtime: " +
            std::to_string(result),
            Logger::Level::Info, Logger::MsgType::Platform
        );
        return nullptr;
    }

    XrInstanceProperties instanceProp{XR_TYPE_INSTANCE_PROPERTIES};
    xrGetInstanceProperties(xrInstance, &instanceProp);

    Logger::Write(
        "[OpenXR] Runtime: " + std::string(instanceProp.runtimeName),
        Logger::Level::Info, Logger::MsgType::Platform
    );

    XrSystemGetInfo systemInfo = {XR_TYPE_SYSTEM_GET_INFO};
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    // Will fail if VR headset is not plugged into the computer
    XrSystemId systemId;
    result = xrGetSystem(xrInstance, &systemInfo, &systemId);

    if (XR_FAILED(result))
    {
        xrResultToString(xrInstance, result, resultBuffer);
        Logger::Write(
            "[OpenXR] Failed to load OpenXR system: " +
            std::string(resultBuffer),
            Logger::Level::Info, Logger::MsgType::Platform
        );
        return nullptr;
    }

    XrSystemProperties systemProp{XR_TYPE_SYSTEM_PROPERTIES};
    xrGetSystemProperties(xrInstance, systemId, &systemProp);

    Logger::Write(
        "[OpenXR] System name: " + std::string(systemProp.systemName),
        Logger::Level::Info, Logger::MsgType::Platform
    );

    // Only allocate an object when system is available
    OpenxrPlatform* platform = new OpenxrPlatform();
    platform->layerList = layerList;
    platform->extensionList = extensionList;
    platform->xrInstance = xrInstance;
    platform->systemId = systemId;

    platform->LoadViewConfig();
    platform->LoadVulkanRequirements();

    return platform;
}

void OpenxrPlatform::LoadViewConfig()
{
    unsigned int count;
    CHK_XRCMD(xrEnumerateViewConfigurations
        (xrInstance, systemId, 0, &count, nullptr));

    viewConfigTypeList.resize(count);
    CHK_XRCMD(xrEnumerateViewConfigurations(
        xrInstance, systemId, count, &count, viewConfigTypeList.data()));
    
    bool configFound = false;
    for (auto& configType: viewConfigTypeList)
    {
        if (configType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO)
        {
            configFound = true;
            break;
        }
    }

    if (!configFound)
        Logger::Write(
            "[OpenXR] The system does not support stereo views",
            Logger::Level::Error, Logger::MsgType::Platform
        );
    
    CHK_XRCMD(xrEnumerateViewConfigurationViews(
        xrInstance, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        0, &count, nullptr));
    
    viewConfigViewList.resize(count, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
    CHK_XRCMD(xrEnumerateViewConfigurationViews(
        xrInstance, systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        count, &count, viewConfigViewList.data()));

    Logger::Write(
        "[OpenXR] Number of views: " + std::to_string(viewConfigViewList.size()) +
        "\n[OpenXR] recommendedImageRectWidth: " + 
        std::to_string(viewConfigViewList[0].recommendedImageRectWidth) + 
        "\n[OpenXR] recommendedImageRectHeight: " + 
        std::to_string(viewConfigViewList[0].recommendedImageRectHeight) + 
        "\n[OpenXR] recommendedSwapchainSampleCount: " + 
        std::to_string(viewConfigViewList[0].recommendedSwapchainSampleCount),
        Logger::Level::Info, Logger::MsgType::Platform
    );
}

void OpenxrPlatform::PrintErrorMsg(XrResult result)
{
    xrResultToString(xrInstance, result, resultBuffer);
    Logger::Write(
        "[OpenXR] API call error: " + std::string(resultBuffer),
        Logger::Level::Error, Logger::MsgType::Platform
    );
}

void OpenxrPlatform::LoadVulkanRequirements()
{
    PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsReqKHR;
    CHK_XRCMD(xrGetInstanceProcAddr(
        xrInstance, "xrGetVulkanGraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanGraphicsReqKHR)));
    xrGetVulkanGraphicsReqKHR(xrInstance, systemId, &vkRequirements);

    PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtKHR;
    CHK_XRCMD(xrGetInstanceProcAddr(
        xrInstance, "xrGetVulkanInstanceExtensionsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanInstanceExtKHR)));

    unsigned int count;
    xrGetVulkanInstanceExtKHR(xrInstance, systemId, 0, &count, nullptr);
    vulkanInstanceExtStr.resize(count);
    xrGetVulkanInstanceExtKHR(
        xrInstance, systemId, count, &count, vulkanInstanceExtStr.data());
    vulkanInstanceExt = ParseExtensionString(vulkanInstanceExtStr.data());

    for(const char* extension: vulkanInstanceExt)
    {
        Logger::Write(
            "[OpenXR] vkInstance extension: " + std::string(extension),
            Logger::Level::Info, Logger::MsgType::Platform
        );
    }

    PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtKHR;
    CHK_XRCMD(xrGetInstanceProcAddr(
        xrInstance, "xrGetVulkanDeviceExtensionsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanDeviceExtKHR)));

    xrGetVulkanDeviceExtKHR(xrInstance, systemId, 0, &count, nullptr);
    vulkanDeviceExtStr.resize(count);
    xrGetVulkanDeviceExtKHR(
        xrInstance, systemId, count, &count, vulkanDeviceExtStr.data());
    vulkanDeviceExt = ParseExtensionString(vulkanDeviceExtStr.data());

    for(const char* extension: vulkanDeviceExt)
    {
        Logger::Write(
            "[OpenXR] vkDevice extension: " + std::string(extension),
            Logger::Level::Info, Logger::MsgType::Platform
        );
    }
}

std::vector<const char*> OpenxrPlatform::ParseExtensionString(char* names)
{
    std::vector<const char*> list;
    while (*names != 0)
    {
        list.push_back(names);
        while (*(++names) != 0)
        {
            if (*names == ' ')
            {
                *names++ = '\0';
                break;
            }
        }
    }
    return list;
}