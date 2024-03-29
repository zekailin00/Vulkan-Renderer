#include "openxr_platform.h"

#include "openxr_swapchain.h"
#include "event_queue.h"
#include "events.h"
#include "logger.h"

#include <vector>
#include <string>
#include <tracy/Tracy.hpp>

#define XR_SUCCEEDED(result) ((result) >= 0)
#define XR_FAILED(result) ((result) < 0)
#define XR_UNQUALIFIED_SUCCESS(result) ((result) == 0)

static XrResult result;
static char resultBuffer[XR_MAX_STRUCTURE_NAME_SIZE];

#define CHK_XRCMD(result)                                                      \
    if (XR_FAILED(result)) {PrintErrorMsg(result);}    



// OSX C standard library does not have strcpy_s
// helper for cross-platform 
namespace utility
{

/**
 * dest buffer size MUST be larger than src buffer size.
 * src MUST be null terminated.
*/
static void strcpy_s(char* dest, const char* src)
{
    int i = 0;
    while(src[i] != 0)
    {
        dest[i] = src[i];
        i++;
    }
    
    dest[i] = 0;
}
 
} // namespace utility
                                                     

XrPlatform* OpenxrPlatform::Initialize()
{
    ZoneScopedN("OpenxrPlatform::Initialize");

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

    std::vector<const char*> enabledExt = {
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
        return new XrPlatformNotFound();
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
    XrSystemId xrSystemId;
    result = xrGetSystem(xrInstance, &systemInfo, &xrSystemId);

    if (XR_FAILED(result))
    {
        xrResultToString(xrInstance, result, resultBuffer);
        Logger::Write(
            "[OpenXR] Failed to load OpenXR system: " +
            std::string(resultBuffer),
            Logger::Level::Info, Logger::MsgType::Platform
        );
        xrDestroyInstance(xrInstance);
        return new XrPlatformNotFound();
    }

    XrSystemProperties systemProp{XR_TYPE_SYSTEM_PROPERTIES};
    xrGetSystemProperties(xrInstance, xrSystemId, &systemProp);

    Logger::Write(
        "[OpenXR] System name: " + std::string(systemProp.systemName),
        Logger::Level::Info, Logger::MsgType::Platform
    );

    // Only allocate an object when system is available
    OpenxrPlatform* platform = new OpenxrPlatform();
    platform->layerList = layerList;
    platform->extensionList = extensionList;
    platform->xrInstance = xrInstance;
    platform->xrSystemId = xrSystemId;

    platform->LoadViewConfig();
    platform->LoadVulkanRequirements();
    platform->InitializeActions();

    return platform;
}

OpenxrSession* OpenxrPlatform::NewSession()
{
    ZoneScopedN("OpenxrPlatform::NewSession");

    session = new OpenxrSession();
    session->SetOpenxrContext(this);
    return session;
}

bool OpenxrPlatform::ShouldCloseSession()
{
    ZoneScopedN("OpenxrPlatform::ShouldCloseSeesion");

    if (!session)
        return false;
    
    if (session->GetSessionState() == XR_SESSION_STATE_EXITING ||
        session->GetSessionState() == XR_SESSION_STATE_LOSS_PENDING)
    {
        session = nullptr;
        return true;
    }

    return false;
}

void OpenxrPlatform::RequestCloseSession()
{
    xrRequestExitSession(session->GetSession());
}

bool OpenxrPlatform::IsSessionRunning()
{
    return !(session == nullptr);
}

void OpenxrPlatform::Destroy()
{
    ZoneScopedN("OpenxrPlatform::Destroy");

    // xrDestroyAction()
    xrDestroyActionSet(inputActionSet);
    xrDestroyInstance(xrInstance);
}

void OpenxrPlatform::PollEvents()
{
    ZoneScopedN("OpenxrPlatform::PollEvents");

    XrEventDataBuffer event{};
    while (TryReadNextEvent(&event))
    {
        switch (event.type)
        {
            case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            {
                const XrEventDataInstanceLossPending& instanceLossPending = 
                    *reinterpret_cast<const XrEventDataInstanceLossPending*>(&event);
                
                //TODO: exit session?
                Logger::Write("[OpenXR] XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING",
                    Logger::Level::Error, Logger::MsgType::Platform);
                break;
            }
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            {
                const XrEventDataSessionStateChanged& sessionStateChangedEvent =
                    *reinterpret_cast<const XrEventDataSessionStateChanged*>(&event);

                if (session)
                    session->SetSessionState(sessionStateChangedEvent.state);
                break;
            }
            case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
            {
                Logger::Write("[OpenXR] XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED",
                    Logger::Level::Info, Logger::MsgType::Platform);
                break;
            }
            case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
            {
                Logger::Write("[OpenXR] XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING",
                    Logger::Level::Info, Logger::MsgType::Platform);
                break;
            }
            case XR_TYPE_EVENT_DATA_EVENTS_LOST:
            {
                const XrEventDataEventsLost& eventsLost =
                    *reinterpret_cast<const XrEventDataEventsLost*>(&event);

                Logger::Write(
                    "[OpenXR] Events lost: " + std::to_string(eventsLost.lostEventCount),
                    Logger::Level::Warning, Logger::MsgType::Platform
                );
            }
            default:
            {
                Logger::Write("[OpenXR] Unknown event type " + std::to_string(event.type),
                    Logger::Level::Info, Logger::MsgType::Platform);
                break;
            }
        }
    }
    return;
}

bool OpenxrPlatform::TryReadNextEvent(XrEventDataBuffer* eventDataBuffer)
{
    ZoneScopedN("OpenxrPlatform::TryReadNextEvent");

    XrEventDataBaseHeader* baseHeader =
        reinterpret_cast<XrEventDataBaseHeader*>(eventDataBuffer);

    *baseHeader = {XR_TYPE_EVENT_DATA_BUFFER};
    const XrResult result = xrPollEvent(xrInstance, eventDataBuffer);
    CHK_XRCMD(result);

    if (result == XR_SUCCESS)
        return true;

    return false;
}

void OpenxrPlatform::BeginFrame()
{
    if (!session) return;

    XrActiveActionSet activeActionSet{inputActionSet, XR_NULL_PATH};
    XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
    syncInfo.countActiveActionSets = 1;
    syncInfo.activeActionSets = &activeActionSet;
    CHK_XRCMD(xrSyncActions(session->GetSession(), &syncInfo));

    session->BeginFrame();


    XrActionStateFloat floatActionState{XR_TYPE_ACTION_STATE_FLOAT};
    XrActionStateBoolean boolActionState{XR_TYPE_ACTION_STATE_BOOLEAN};
    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};

    getInfo.action = lSqueezeValueAction;
    CHK_XRCMD(xrGetActionStateFloat(
        session->GetSession(), &getInfo, &floatActionState));
    if (floatActionState.isActive)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::LEFT_SQUEEZE;
        event->floatValue = floatActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = rSqueezeValueAction;
    CHK_XRCMD(xrGetActionStateFloat(
        session->GetSession(), &getInfo, &floatActionState));
    if (floatActionState.isActive)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::RIGHT_SQUEEZE;
        event->floatValue = floatActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = lTriggerValueAction;
    CHK_XRCMD(xrGetActionStateFloat(
        session->GetSession(), &getInfo, &floatActionState));
    if (floatActionState.isActive)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::LEFT_TRIGGER;
        event->floatValue = floatActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = rTriggerValueAction;
    CHK_XRCMD(xrGetActionStateFloat(
        session->GetSession(), &getInfo, &floatActionState));
    if (floatActionState.isActive)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::RIGHT_TRIGGER;
        event->floatValue = floatActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = lThumbstickXAction;
    CHK_XRCMD(xrGetActionStateFloat(
        session->GetSession(), &getInfo, &floatActionState));
    if (floatActionState.isActive)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::LEFT_THUMBSTICK_X;
        event->floatValue = floatActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = rThumbstickXAction;
    CHK_XRCMD(xrGetActionStateFloat(
        session->GetSession(), &getInfo, &floatActionState));
    if (floatActionState.isActive)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::RIGHT_THUMBSTICK_X;
        event->floatValue = floatActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = lThumbstickYAction;
    CHK_XRCMD(xrGetActionStateFloat(
        session->GetSession(), &getInfo, &floatActionState));
    if (floatActionState.isActive)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::LEFT_THUMBSTICK_Y;
        event->floatValue = floatActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = rThumbstickYAction;
    CHK_XRCMD(xrGetActionStateFloat(
        session->GetSession(), &getInfo, &floatActionState));
    if (floatActionState.isActive)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::RIGHT_THUMBSTICK_Y;
        event->floatValue = floatActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = lThumbstickClickAction;
    CHK_XRCMD(xrGetActionStateBoolean(
        session->GetSession(), &getInfo, &boolActionState));
    if (boolActionState.isActive && boolActionState.changedSinceLastSync)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::LEFT_THUMBSTICK_CLICK;
        event->boolValue = boolActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = rThumbstickClickAction;
    CHK_XRCMD(xrGetActionStateBoolean(
        session->GetSession(), &getInfo, &boolActionState));
    if (boolActionState.isActive && boolActionState.changedSinceLastSync)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::RIGHT_THUMBSTICK_CLICK;
        event->boolValue = boolActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = lXClickAction;
    CHK_XRCMD(xrGetActionStateBoolean(
        session->GetSession(), &getInfo, &boolActionState));
    if (boolActionState.isActive && boolActionState.changedSinceLastSync)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::LEFT_X_CLICK;
        event->boolValue = boolActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = lYClickAction;
    CHK_XRCMD(xrGetActionStateBoolean(
        session->GetSession(), &getInfo, &boolActionState));
    if (boolActionState.isActive && boolActionState.changedSinceLastSync)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::LEFT_Y_CLICK;
        event->boolValue = boolActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = lMenuClickAction;
    CHK_XRCMD(xrGetActionStateBoolean(
        session->GetSession(), &getInfo, &boolActionState));
    if (boolActionState.isActive && boolActionState.changedSinceLastSync)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::LEFT_MENU_CLICK;
        event->boolValue = boolActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = rAClickAction;
    CHK_XRCMD(xrGetActionStateBoolean(
        session->GetSession(), &getInfo, &boolActionState));
    if (boolActionState.isActive && boolActionState.changedSinceLastSync)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::RIGHT_A_CLICK;
        event->boolValue = boolActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }

    getInfo.action = rBClickAction;
    CHK_XRCMD(xrGetActionStateBoolean(
        session->GetSession(), &getInfo, &boolActionState));
    if (boolActionState.isActive && boolActionState.changedSinceLastSync)
    {
        EventControllerInput* event = new EventControllerInput();
        event->keycode = KeyCode::RIGHT_B_CLICK;
        event->boolValue = boolActionState.currentState;

        EventQueue::GetInstance()->Publish(
            EventQueue::InputXR, event
        );
    }
}

void OpenxrPlatform::EndFrame()
{
    if (!session) return;
    session->EndFrame();
}

void OpenxrPlatform::InitializeActions()
{
    // Create an action set
    XrActionSetCreateInfo actionSetInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
    utility::strcpy_s(actionSetInfo.actionSetName, "input");
    utility::strcpy_s(actionSetInfo.localizedActionSetName, "input");
    actionSetInfo.priority = 0;
    CHK_XRCMD(xrCreateActionSet(xrInstance, &actionSetInfo, &inputActionSet));

    XrActionCreateInfo actioninfo{XR_TYPE_ACTION_CREATE_INFO};

    utility::strcpy_s(actioninfo.actionName, "l_squeeze_value");
    utility::strcpy_s(actioninfo.localizedActionName, "l_squeeze_value");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lSqueezeValueAction));

    utility::strcpy_s(actioninfo.actionName, "r_squeeze_value");
    utility::strcpy_s(actioninfo.localizedActionName, "rSqueezeValue");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rSqueezeValueAction));

    utility::strcpy_s(actioninfo.actionName, "l_trigger_value");
    utility::strcpy_s(actioninfo.localizedActionName, "l_trigger_value");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lTriggerValueAction));

    utility::strcpy_s(actioninfo.actionName, "r_trigger_value");
    utility::strcpy_s(actioninfo.localizedActionName, "r_trigger_value");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rTriggerValueAction));

    utility::strcpy_s(actioninfo.actionName, "l_trigger_touch");
    utility::strcpy_s(actioninfo.localizedActionName, "l_trigger_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lTriggerTouchAction));

    utility::strcpy_s(actioninfo.actionName, "r_trigger_touch");
    utility::strcpy_s(actioninfo.localizedActionName, "r_trigger_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rTriggerTouchAction));

    utility::strcpy_s(actioninfo.actionName, "l_thumbstick_x");
    utility::strcpy_s(actioninfo.localizedActionName, "l_thumbstick_x");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lThumbstickXAction));

    utility::strcpy_s(actioninfo.actionName, "r_thumbstick_x");
    utility::strcpy_s(actioninfo.localizedActionName, "r_thumbstick_x");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rThumbstickXAction));

    utility::strcpy_s(actioninfo.actionName, "l_thumbstick_y");
    utility::strcpy_s(actioninfo.localizedActionName, "l_thumbstick_y");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lThumbstickYAction));

    utility::strcpy_s(actioninfo.actionName, "r_thumbstick_y");
    utility::strcpy_s(actioninfo.localizedActionName, "r_thumbstick_y");
    actioninfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rThumbstickYAction));

    utility::strcpy_s(actioninfo.actionName, "l_thumbstick_click");
    utility::strcpy_s(actioninfo.localizedActionName, "l_thumbstick_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lThumbstickClickAction));

    utility::strcpy_s(actioninfo.actionName, "r_thumbstick_click");
    utility::strcpy_s(actioninfo.localizedActionName, "r_thumbstick_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rThumbstickClickAction));

    utility::strcpy_s(actioninfo.actionName, "l_thumbstick_touch");
    utility::strcpy_s(actioninfo.localizedActionName, "l_thumbstick_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lThumbstickTouchAction));

    utility::strcpy_s(actioninfo.actionName, "r_thumbstick_touch");
    utility::strcpy_s(actioninfo.localizedActionName, "r_thumbstick_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rThumbstickTouchAction));

    utility::strcpy_s(actioninfo.actionName, "l_x_click");
    utility::strcpy_s(actioninfo.localizedActionName, "l_x_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lXClickAction));

    utility::strcpy_s(actioninfo.actionName, "l_x_touch");
    utility::strcpy_s(actioninfo.localizedActionName, "l_x_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lXTouchAction));

    utility::strcpy_s(actioninfo.actionName, "l_y_click");
    utility::strcpy_s(actioninfo.localizedActionName, "l_y_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lYClickAction));

    utility::strcpy_s(actioninfo.actionName, "l_y_touch");
    utility::strcpy_s(actioninfo.localizedActionName, "l_y_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lYTouchAction));

    utility::strcpy_s(actioninfo.actionName, "l_menu_click");
    utility::strcpy_s(actioninfo.localizedActionName, "l_menu_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lMenuClickAction));

    utility::strcpy_s(actioninfo.actionName, "r_a_click");
    utility::strcpy_s(actioninfo.localizedActionName, "r_a_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rAClickAction));

    utility::strcpy_s(actioninfo.actionName, "r_a_touch");
    utility::strcpy_s(actioninfo.localizedActionName, "r_a_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rATouchAction));

    utility::strcpy_s(actioninfo.actionName, "r_b_click");
    utility::strcpy_s(actioninfo.localizedActionName, "r_b_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rBClickAction));

    utility::strcpy_s(actioninfo.actionName, "r_b_touch");
    utility::strcpy_s(actioninfo.localizedActionName, "r_b_touch");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rBTouchAction));

    utility::strcpy_s(actioninfo.actionName, "r_system_click");
    utility::strcpy_s(actioninfo.localizedActionName, "r_system_click");
    actioninfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rSystemClickAction));

    utility::strcpy_s(actioninfo.actionName, "l_grip_pose");
    utility::strcpy_s(actioninfo.localizedActionName, "l_grip_pose");
    actioninfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lGripPoseAction));

    utility::strcpy_s(actioninfo.actionName, "r_grip_pose");
    utility::strcpy_s(actioninfo.localizedActionName, "r_grip_pose");
    actioninfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rGripPoseAction));

    utility::strcpy_s(actioninfo.actionName, "l_aim_pose");
    utility::strcpy_s(actioninfo.localizedActionName, "l_aim_pose");
    actioninfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &lAimPoseAction));

    utility::strcpy_s(actioninfo.actionName, "r_aim_pose");
    utility::strcpy_s(actioninfo.localizedActionName, "r_aim_pose");
    actioninfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
    CHK_XRCMD(xrCreateAction(inputActionSet, &actioninfo, &rAimPoseAction));

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

    XrPath lGripPosePath;
    XrPath rGripPosePath;
    XrPath lAimPosePath;
    XrPath rAimPosePath;

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

    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/grip/pose", &lGripPosePath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/grip/pose", &rGripPosePath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/left/input/aim/pose", &lAimPosePath));
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/user/hand/right/input/aim/pose", &rAimPosePath));

    XrActionSuggestedBinding bindings[28];
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

    bindings[24].action = lGripPoseAction;
    bindings[24].binding = lGripPosePath;
    bindings[25].action = rGripPoseAction;
    bindings[25].binding = rGripPosePath;
    bindings[26].action = lAimPoseAction;
    bindings[26].binding = lAimPosePath;
    bindings[27].action = rAimPoseAction;
    bindings[27].binding = rAimPosePath;

    XrPath oculusTouchInteractionProfilePath;
    CHK_XRCMD(xrStringToPath(xrInstance,
        "/interaction_profiles/oculus/touch_controller",
        &oculusTouchInteractionProfilePath
    ));

    XrInteractionProfileSuggestedBinding suggestedBindings{
        XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindings.interactionProfile = oculusTouchInteractionProfilePath;
    suggestedBindings.suggestedBindings = bindings;
    suggestedBindings.countSuggestedBindings = 28;
    CHK_XRCMD(xrSuggestInteractionProfileBindings(
        xrInstance, &suggestedBindings));
    
}

void OpenxrPlatform::LoadViewConfig()
{
    unsigned int count;
    CHK_XRCMD(xrEnumerateViewConfigurations
        (xrInstance, xrSystemId, 0, &count, nullptr));

    viewConfigTypeList.resize(count);
    CHK_XRCMD(xrEnumerateViewConfigurations(
        xrInstance, xrSystemId, count, &count, viewConfigTypeList.data()));
    
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
        xrInstance, xrSystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        0, &count, nullptr));
    
    viewConfigViewList.resize(count, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
    CHK_XRCMD(xrEnumerateViewConfigurationViews(
        xrInstance, xrSystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
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
    xrGetVulkanGraphicsReqKHR(xrInstance, xrSystemId, &vkRequirements);

    PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtKHR;
    CHK_XRCMD(xrGetInstanceProcAddr(
        xrInstance, "xrGetVulkanInstanceExtensionsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanInstanceExtKHR)));

    unsigned int count;
    xrGetVulkanInstanceExtKHR(xrInstance, xrSystemId, 0, &count, nullptr);
    vulkanInstanceExtStr.resize(count);
    xrGetVulkanInstanceExtKHR(
        xrInstance, xrSystemId, count, &count, vulkanInstanceExtStr.data());
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

    xrGetVulkanDeviceExtKHR(xrInstance, xrSystemId, 0, &count, nullptr);
    vulkanDeviceExtStr.resize(count);
    xrGetVulkanDeviceExtKHR(
        xrInstance, xrSystemId, count, &count, vulkanDeviceExtStr.data());
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