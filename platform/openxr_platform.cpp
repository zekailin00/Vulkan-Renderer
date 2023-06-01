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
    platform->InitializeActions();

    return platform;
}

void OpenxrPlatform::InitializeSession(VulkanDevice* vulkanDevice)
{
    XrGraphicsBindingVulkanKHR vkBinding{XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR};
    vkBinding.instance = vulkanDevice->vkInstance;
    vkBinding.physicalDevice = vulkanDevice->vkPhysicalDevice;
    vkBinding.device = vulkanDevice->vkDevice;
    vkBinding.queueFamilyIndex = vulkanDevice->graphicsIndex;
    vkBinding.queueIndex = 0;

    PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR;
    CHK_XRCMD(xrGetInstanceProcAddr(
        xrInstance, "xrGetVulkanGraphicsDeviceKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanGraphicsDeviceKHR)));

    // Ensure the physical device used by Vulkan renderer has a VR headset connected.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    xrGetVulkanGraphicsDeviceKHR(
        xrInstance, systemId, vulkanDevice->vkInstance, &physicalDevice);
    if (physicalDevice != vulkanDevice->vkPhysicalDevice)
    {
        //TODO:
        throw;
    }

    //FIXME:  vkCreateShaderModule() vulkan validation error
    XrSessionCreateInfo createInfo{XR_TYPE_SESSION_CREATE_INFO};
    createInfo.next = &vkBinding;
    createInfo.systemId = systemId;
    CHK_XRCMD(xrCreateSession(xrInstance, &createInfo, &xrSession));

    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &inputActionSet;
    CHK_XRCMD(xrAttachSessionActionSets(xrSession, &attachInfo));

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

    {   // enumerate reference spaces
        uint32_t spaceCount;
        CHK_XRCMD(xrEnumerateReferenceSpaces(xrSession, 0, &spaceCount, nullptr));
        std::vector<XrReferenceSpaceType> spaces(spaceCount);
        CHK_XRCMD(xrEnumerateReferenceSpaces(xrSession, spaceCount, &spaceCount, spaces.data()));

        Logger::Write(
            "Available reference spaces: " + std::to_string(spaceCount),
            Logger::Level::Info, Logger::MsgType::Platform
        );

        for (XrReferenceSpaceType space : spaces) {
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
                spaceName = "Enum number: " + std::to_string(space);
                break;
            }
            Logger::Write(spaceName,
                Logger::Level::Info, Logger::MsgType::Platform);
        }
    }

    
}

void OpenxrPlatform::InitializeActions()
{
    // Create an action set
    XrActionSetCreateInfo actionSetInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
    strcpy_s(actionSetInfo.actionSetName, "input");
    strcpy_s(actionSetInfo.localizedActionSetName, "input");
    actionSetInfo.priority = 0;
    CHK_XRCMD(xrCreateActionSet(xrInstance, &actionSetInfo, &inputActionSet));
    
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

    XrActionCreateInfo actioninfo{XR_TYPE_ACTION_CREATE_INFO};

    strcpy_s(actioninfo.actionName, "l_squeeze_value");
    strcpy_s(actioninfo.localizedActionName, "l_squeeze_value");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lSqueezeValueAction));

    strcpy_s(actioninfo.actionName, "r_squeeze_value");
    strcpy_s(actioninfo.localizedActionName, "rSqueezeValue");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rSqueezeValueAction));

    strcpy_s(actioninfo.actionName, "l_trigger_value");
    strcpy_s(actioninfo.localizedActionName, "l_trigger_value");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lTriggerValueAction));

    strcpy_s(actioninfo.actionName, "r_trigger_value");
    strcpy_s(actioninfo.localizedActionName, "r_trigger_value");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rTriggerValueAction));

    strcpy_s(actioninfo.actionName, "l_trigger_touch");
    strcpy_s(actioninfo.localizedActionName, "l_trigger_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lTriggerTouchAction));

    strcpy_s(actioninfo.actionName, "r_trigger_touch");
    strcpy_s(actioninfo.localizedActionName, "r_trigger_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rTriggerTouchAction));

    strcpy_s(actioninfo.actionName, "l_thumbstick_x");
    strcpy_s(actioninfo.localizedActionName, "l_thumbstick_x");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lThumbstickXAction));

    strcpy_s(actioninfo.actionName, "r_thumbstick_x");
    strcpy_s(actioninfo.localizedActionName, "r_thumbstick_x");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rThumbstickXAction));

    strcpy_s(actioninfo.actionName, "l_thumbstick_y");
    strcpy_s(actioninfo.localizedActionName, "l_thumbstick_y");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lThumbstickYAction));

    strcpy_s(actioninfo.actionName, "r_thumbstick_y");
    strcpy_s(actioninfo.localizedActionName, "r_thumbstick_y");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rThumbstickYAction));

    strcpy_s(actioninfo.actionName, "l_thumbstick_click");
    strcpy_s(actioninfo.localizedActionName, "l_thumbstick_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lThumbstickClickAction));

    strcpy_s(actioninfo.actionName, "r_thumbstick_click");
    strcpy_s(actioninfo.localizedActionName, "r_thumbstick_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rThumbstickClickAction));

    strcpy_s(actioninfo.actionName, "l_thumbstick_touch");
    strcpy_s(actioninfo.localizedActionName, "l_thumbstick_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lThumbstickTouchAction));

    strcpy_s(actioninfo.actionName, "r_thumbstick_touch");
    strcpy_s(actioninfo.localizedActionName, "r_thumbstick_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rThumbstickTouchAction));

    strcpy_s(actioninfo.actionName, "l_x_click");
    strcpy_s(actioninfo.localizedActionName, "l_x_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lXClickAction));

    strcpy_s(actioninfo.actionName, "l_x_touch");
    strcpy_s(actioninfo.localizedActionName, "l_x_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lXTouchAction));

    strcpy_s(actioninfo.actionName, "l_y_click");
    strcpy_s(actioninfo.localizedActionName, "l_y_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lYClickAction));

    strcpy_s(actioninfo.actionName, "l_y_touch");
    strcpy_s(actioninfo.localizedActionName, "l_y_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lYTouchAction));

    strcpy_s(actioninfo.actionName, "l_menu_click");
    strcpy_s(actioninfo.localizedActionName, "l_menu_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lMenuClickAction));

    strcpy_s(actioninfo.actionName, "r_a_click");
    strcpy_s(actioninfo.localizedActionName, "r_a_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rAClickAction));

    strcpy_s(actioninfo.actionName, "r_a_touch");
    strcpy_s(actioninfo.localizedActionName, "r_a_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rATouchAction));

    strcpy_s(actioninfo.actionName, "r_b_click");
    strcpy_s(actioninfo.localizedActionName, "r_b_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rBClickAction));

    strcpy_s(actioninfo.actionName, "r_b_touch");
    strcpy_s(actioninfo.localizedActionName, "r_b_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rBTouchAction));

    strcpy_s(actioninfo.actionName, "r_system_click");
    strcpy_s(actioninfo.localizedActionName, "r_system_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rSystemClickAction));

    XrPath lSqueezeValuePath;
    XrPath rSqueezeValuePath;

    XrPath lTriggerValuePath;
    XrPath rTriggerValuePath;

    XrPath lTriggerTouchPath;
    XrPath rTriggerTouchPath;

    XrPath lThumbstickXPath;
    XrPath rThumbstickXPath;

    XrPath lThumbstickYPath;
    XrPath rThumbstickYPath;

    XrPath lThumbstickClickPath;
    XrPath rThumbstickClickPath;

    XrPath lThumbstickTouchPath;
    XrPath rThumbstickTouchPath;

    XrPath lXClickPath;
    XrPath lXTouchPath;
    XrPath lYClickPath;
    XrPath lYTouchPath;
    XrPath lMenuClickPath;

    XrPath rAClickPath;
    XrPath rATouchPath;
    XrPath rBClickPath;
    XrPath rBTouchPath;
    XrPath rSystemClickPath;

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/squeeze/value", &lSqueezeValuePath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/squeeze/value", &rSqueezeValuePath));

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/trigger/value", &lTriggerValuePath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/trigger/value", &rTriggerValuePath));

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/trigger/touch", &lTriggerTouchPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/trigger/touch", &rTriggerTouchPath));

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/thumbstick/x", &lThumbstickXPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/thumbstick/x", &rThumbstickXPath));

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/thumbstick/y", &lThumbstickYPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/thumbstick/y", &rThumbstickYPath));

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/thumbstick/click", &lThumbstickClickPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/thumbstick/click", &rThumbstickClickPath));

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/thumbstick/touch", &lThumbstickTouchPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/thumbstick/touch", &rThumbstickTouchPath));

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/x/click", &lXClickPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/x/touch", &lXTouchPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/y/click", &lYClickPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/y/touch", &lYTouchPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/menu/click", &lMenuClickPath));

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/a/click", &rAClickPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/a/touch", &rATouchPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/b/click", &rBClickPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/b/touch", &rBTouchPath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/system/click", &rSystemClickPath));

    XrActionSuggestedBinding bindings[24];
    bindings[0].action = lSqueezeValueAction;
    bindings[0].binding = lSqueezeValuePath;
    bindings[1].action = rSqueezeValueAction;
    bindings[1].binding = rSqueezeValuePath;

    bindings[2].action = lTriggerValueAction;
    bindings[2].binding = lTriggerValuePath;
    bindings[3].action = rTriggerValueAction;
    bindings[3].binding = rTriggerValuePath;

    bindings[4].action = lTriggerTouchAction;
    bindings[4].binding = lTriggerTouchPath;
    bindings[5].action = rTriggerTouchAction;
    bindings[5].binding = rTriggerTouchPath;

    bindings[6].action = lThumbstickXAction;
    bindings[6].binding = lThumbstickXPath;
    bindings[7].action = rThumbstickXAction;
    bindings[7].binding = rThumbstickXPath;

    bindings[8].action = lThumbstickYAction;
    bindings[8].binding = lThumbstickYPath;
    bindings[9].action = rThumbstickYAction;
    bindings[9].binding = rThumbstickYPath;

    bindings[10].action = lThumbstickClickAction;
    bindings[10].binding = lThumbstickClickPath;
    bindings[11].action = rThumbstickClickAction;
    bindings[11].binding = rThumbstickClickPath;

    bindings[12].action = lThumbstickTouchAction;
    bindings[12].binding = lThumbstickTouchPath;
    bindings[13].action = rThumbstickTouchAction;
    bindings[13].binding = rThumbstickTouchPath;

    bindings[14].action = lXClickAction;
    bindings[14].binding = lXClickPath;
    bindings[15].action = lXTouchAction;
    bindings[15].binding = lXTouchPath;
    bindings[16].action = lYClickAction;
    bindings[16].binding = lYClickPath;
    bindings[17].action = lYTouchAction;
    bindings[17].binding = lYTouchPath;
    bindings[18].action = lMenuClickAction;
    bindings[18].binding = lMenuClickPath;

    bindings[19].action = rAClickAction;
    bindings[19].binding = rAClickPath;
    bindings[20].action = rATouchAction;
    bindings[20].binding = rATouchPath;
    bindings[21].action = rBClickAction;
    bindings[21].binding = rBClickPath;
    bindings[22].action = rBTouchAction;
    bindings[22].binding = rBTouchPath;
    bindings[23].action = rSystemClickAction;
    bindings[23].binding = rSystemClickPath;

    XrPath oculusTouchInteractionProfilePath;
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/interaction_profiles/oculus/touch_controller",
        &oculusTouchInteractionProfilePath
    ));

    XrInteractionProfileSuggestedBinding suggestedBindings{
        XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindings.interactionProfile = oculusTouchInteractionProfilePath;
    suggestedBindings.suggestedBindings = bindings;
    suggestedBindings.countSuggestedBindings = 24;
    CHK_XRCMD(xrSuggestInteractionProfileBindings(
        xrInstance, &suggestedBindings));
    
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
    vkRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;
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