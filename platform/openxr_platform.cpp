#include "openxr_platform.h"

#include "logger.h"

#include <vector>
#include <string>

#define XR_SUCCEEDED(result) ((result) >= 0)
#define XR_FAILED(result) ((result) < 0)
#define XR_UNQUALIFIED_SUCCESS(result) ((result) == 0)

static XrResult result;
static char resultBuffer[XR_MAX_STRUCTURE_NAME_SIZE];

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

    // Only allocate an object when system is available
    OpenxrPlatform* platform = new OpenxrPlatform();
    platform->layerList = layerList;
    platform->extensionList = extensionList;
    platform->xrInstance = xrInstance;
    platform->systemId = systemId;

    return platform;
}