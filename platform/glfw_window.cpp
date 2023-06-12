#include "glfw_window.h"

#include "vulkan_renderer.h"
#include "validation.h"
#include "logger.h"
#include "event_queue.h"
#include "events.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <tracy/Tracy.hpp>

#define GLFW_VERSION_COMBINED (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 + GLFW_VERSION_REVISION)
#define GLFW_HAS_GETKEYNAME   (GLFW_VERSION_COMBINED >= 3200) 
#define GLFW_HAS_GETERROR     (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetError()

// ImGui keyCode conversion for glfw3
static int ImGui_ImplGlfw_TranslateUntranslatedKey(int key, int scancode);
static ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int key);

void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;

    EventMousePosition* event = new EventMousePosition();
    event->pos = {x, y};
    EventQueue::GetInstance()->Publish(EventQueue::InputGFLW, event);
}

void CursorEnterCallback(GLFWwindow* window, int entered)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;

    EventCursorEnter* event = new EventCursorEnter();
    event->entered = entered;
    EventQueue::GetInstance()->Publish(EventQueue::InputGFLW, event);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT ||
        button == GLFW_MOUSE_BUTTON_RIGHT ||
        button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        EventMouseButton* event = new EventMouseButton();
        event->button = (EventMouseButton::Button)button;
        event->action = (EventMouseButton::Action)action;
        EventQueue::GetInstance()->Publish(EventQueue::InputGFLW, event);
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    EventMouseWheel* event = new EventMouseWheel();
    event->offset = {xoffset, yoffset};

    EventQueue::GetInstance()->Publish(EventQueue::InputGFLW, event);
}

void KeyCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
    EventKeyboard* event = new EventKeyboard();
    event->keyCode = keycode;
    event->action = (EventKeyboard::Action)action;

    EventQueue::GetInstance()->Publish(EventQueue::InputGFLW, event);

    // ImGui specific keycode encoding conversion.
    if (action == GLFW_PRESS || action == GLFW_RELEASE)
    {
        keycode = ImGui_ImplGlfw_TranslateUntranslatedKey(keycode, scancode);
        ImGuiKey imguiKey = ImGui_ImplGlfw_KeyToImGuiKey(keycode);

        EventKeyboardImgui* event = new EventKeyboardImgui();
        event->keyCode = imguiKey;
        event->pressed = action == GLFW_PRESS;

        EventQueue::GetInstance()->Publish(EventQueue::InputGFLW, event);
    }
}

void CharCallback(GLFWwindow* window, unsigned int c)
{
    EventCharacterUTF32* event = new EventCharacterUTF32();
    event->c = c;

    EventQueue::GetInstance()->Publish(EventQueue::InputGFLW, event);
}


void GlfwWindow::InitializeWindow()
{
    ZoneScopedN("GlfwWindow::InitializeWindow");

    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit() || !glfwVulkanSupported())
    {
        Logger::Write(
            "[GLFW Window] Initialization failed.",
            Logger::Level::Error,
            Logger::MsgType::Platform
        );
    }

    uint32_t extensionsCount;
    const char** extensions;
    extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
    vkInstanceExt.resize(extensionsCount);
    memcpy(vkInstanceExt.data(), extensions, extensionsCount * sizeof(const char*));

    Logger::Write(
            "[GLFW Window] Number of extensions needed: " + std::to_string(extensionsCount),
            Logger::Level::Info,
            Logger::MsgType::Platform
        );
    
    for (const char* extension: vkInstanceExt)
    {
        Logger::Write(
            "[GLFW Window] vkInstance extension: " + std::string(extension),
            Logger::Level::Info, Logger::MsgType::Platform
        );
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(1280, 720, "Vulkan Renderer", NULL, NULL);

    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetCursorEnterCallback(window, CursorEnterCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCharCallback(window, CharCallback);
}

void GlfwWindow::InitializeSurface()
{
    ZoneScopedN("GlfwWindow::InitializeSurface");

    renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();
    CHECK_VKCMD(glfwCreateWindowSurface(vkr.vkInstance, window, nullptr, &surface));

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, glfwFramebufferResizeCallback);

    VkBool32 result;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkr.vulkanDevice.vkPhysicalDevice, vkr.vulkanDevice.graphicsIndex, surface, &result);
    if (result != VK_TRUE)
    {
        Logger::Write(
            "[GLFW Window] Error: no WSI support on physical device.",
            Logger::Level::Error,
            Logger::MsgType::Platform
        );
    }

    windowSwapchain = new WindowSwapchain();
    windowSwapchain->SetSurface(surface);
}

void GlfwWindow::RegisterPeripherals()
{
    // Nothing
}

bool GlfwWindow::ShouldClose()
{
    ZoneScopedN("GlfwWindow::ShouldClose");
    return glfwWindowShouldClose(window);
}

void GlfwWindow::BeginFrame()
{
    ZoneScopedN("GlfwWindow::BeginFrame");
    
    glfwPollEvents();

    // Rebuild swapchain when window size changes
    if (windowSwapchain->swapchainRebuild)
    {
        // Stall the program if window is minimized.
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) 
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        renderer::VulkanRenderer::GetInstance().RebuildSwapchain();
        windowSwapchain->swapchainRebuild = false;
    }  
}

void GlfwWindow::DestroySurface()
{
    ZoneScopedN("GlfwWindow::DestroySurface");

    delete windowSwapchain;

    // Destory surface before vkInstance
    renderer::VulkanRenderer& vkr = renderer::VulkanRenderer::GetInstance();
    vkDestroySurfaceKHR(vkr.vkInstance, surface, nullptr);
}

void GlfwWindow::DestroyWindow()
{
    ZoneScopedN("GlfwWindow::DestroyWindow");

    glfwSetMouseButtonCallback(window, nullptr);
    glfwSetCursorPosCallback(window, nullptr);
    glfwSetScrollCallback(window, nullptr);
    glfwSetCursorEnterCallback(window, nullptr);
    glfwSetKeyCallback(window, nullptr);
    glfwSetCharCallback(window, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

static int ImGui_ImplGlfw_TranslateUntranslatedKey(int key, int scancode)
{
#if GLFW_HAS_GETKEYNAME
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_EQUAL)
        return key;
    GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
    const char* key_name = glfwGetKeyName(key, scancode);
    glfwSetErrorCallback(prev_error_callback);
#if GLFW_HAS_GETERROR // Eat errors (see #5908)
    (void)glfwGetError(nullptr);
#endif
    if (key_name && key_name[0] != 0 && key_name[1] == 0)
    {
        const char char_names[] = "`-=[]\\,;\'./";
        const int char_keys[] = {
            GLFW_KEY_GRAVE_ACCENT, GLFW_KEY_MINUS,
            GLFW_KEY_EQUAL, GLFW_KEY_LEFT_BRACKET,
            GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH,
            GLFW_KEY_COMMA, GLFW_KEY_SEMICOLON,
            GLFW_KEY_APOSTROPHE, GLFW_KEY_PERIOD, GLFW_KEY_SLASH, 0
        };
        IM_ASSERT(IM_ARRAYSIZE(char_names) == IM_ARRAYSIZE(char_keys));
        if (key_name[0] >= '0' && key_name[0] <= '9')
        { key = GLFW_KEY_0 + (key_name[0] - '0'); }
        else if (key_name[0] >= 'A' && key_name[0] <= 'Z')
        { key = GLFW_KEY_A + (key_name[0] - 'A'); }
        else if (key_name[0] >= 'a' && key_name[0] <= 'z')
        { key = GLFW_KEY_A + (key_name[0] - 'a'); }
        else if (const char* p = strchr(char_names, key_name[0]))
        { key = char_keys[p - char_names]; }
    }
#else
    IM_UNUSED(scancode);
#endif
    return key;
}

static ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int key)
{
    switch (key)
    {
        case GLFW_KEY_TAB: return ImGuiKey_Tab;
        case GLFW_KEY_LEFT: return ImGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT: return ImGuiKey_RightArrow;
        case GLFW_KEY_UP: return ImGuiKey_UpArrow;
        case GLFW_KEY_DOWN: return ImGuiKey_DownArrow;
        case GLFW_KEY_PAGE_UP: return ImGuiKey_PageUp;
        case GLFW_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
        case GLFW_KEY_HOME: return ImGuiKey_Home;
        case GLFW_KEY_END: return ImGuiKey_End;
        case GLFW_KEY_INSERT: return ImGuiKey_Insert;
        case GLFW_KEY_DELETE: return ImGuiKey_Delete;
        case GLFW_KEY_BACKSPACE: return ImGuiKey_Backspace;
        case GLFW_KEY_SPACE: return ImGuiKey_Space;
        case GLFW_KEY_ENTER: return ImGuiKey_Enter;
        case GLFW_KEY_ESCAPE: return ImGuiKey_Escape;
        case GLFW_KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
        case GLFW_KEY_COMMA: return ImGuiKey_Comma;
        case GLFW_KEY_MINUS: return ImGuiKey_Minus;
        case GLFW_KEY_PERIOD: return ImGuiKey_Period;
        case GLFW_KEY_SLASH: return ImGuiKey_Slash;
        case GLFW_KEY_SEMICOLON: return ImGuiKey_Semicolon;
        case GLFW_KEY_EQUAL: return ImGuiKey_Equal;
        case GLFW_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case GLFW_KEY_BACKSLASH: return ImGuiKey_Backslash;
        case GLFW_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
        case GLFW_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
        case GLFW_KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
        case GLFW_KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case GLFW_KEY_NUM_LOCK: return ImGuiKey_NumLock;
        case GLFW_KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case GLFW_KEY_PAUSE: return ImGuiKey_Pause;
        case GLFW_KEY_KP_0: return ImGuiKey_Keypad0;
        case GLFW_KEY_KP_1: return ImGuiKey_Keypad1;
        case GLFW_KEY_KP_2: return ImGuiKey_Keypad2;
        case GLFW_KEY_KP_3: return ImGuiKey_Keypad3;
        case GLFW_KEY_KP_4: return ImGuiKey_Keypad4;
        case GLFW_KEY_KP_5: return ImGuiKey_Keypad5;
        case GLFW_KEY_KP_6: return ImGuiKey_Keypad6;
        case GLFW_KEY_KP_7: return ImGuiKey_Keypad7;
        case GLFW_KEY_KP_8: return ImGuiKey_Keypad8;
        case GLFW_KEY_KP_9: return ImGuiKey_Keypad9;
        case GLFW_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case GLFW_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
        case GLFW_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
        case GLFW_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
        case GLFW_KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case GLFW_KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
        case GLFW_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
        case GLFW_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
        case GLFW_KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
        case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
        case GLFW_KEY_MENU: return ImGuiKey_Menu;
        case GLFW_KEY_0: return ImGuiKey_0;
        case GLFW_KEY_1: return ImGuiKey_1;
        case GLFW_KEY_2: return ImGuiKey_2;
        case GLFW_KEY_3: return ImGuiKey_3;
        case GLFW_KEY_4: return ImGuiKey_4;
        case GLFW_KEY_5: return ImGuiKey_5;
        case GLFW_KEY_6: return ImGuiKey_6;
        case GLFW_KEY_7: return ImGuiKey_7;
        case GLFW_KEY_8: return ImGuiKey_8;
        case GLFW_KEY_9: return ImGuiKey_9;
        case GLFW_KEY_A: return ImGuiKey_A;
        case GLFW_KEY_B: return ImGuiKey_B;
        case GLFW_KEY_C: return ImGuiKey_C;
        case GLFW_KEY_D: return ImGuiKey_D;
        case GLFW_KEY_E: return ImGuiKey_E;
        case GLFW_KEY_F: return ImGuiKey_F;
        case GLFW_KEY_G: return ImGuiKey_G;
        case GLFW_KEY_H: return ImGuiKey_H;
        case GLFW_KEY_I: return ImGuiKey_I;
        case GLFW_KEY_J: return ImGuiKey_J;
        case GLFW_KEY_K: return ImGuiKey_K;
        case GLFW_KEY_L: return ImGuiKey_L;
        case GLFW_KEY_M: return ImGuiKey_M;
        case GLFW_KEY_N: return ImGuiKey_N;
        case GLFW_KEY_O: return ImGuiKey_O;
        case GLFW_KEY_P: return ImGuiKey_P;
        case GLFW_KEY_Q: return ImGuiKey_Q;
        case GLFW_KEY_R: return ImGuiKey_R;
        case GLFW_KEY_S: return ImGuiKey_S;
        case GLFW_KEY_T: return ImGuiKey_T;
        case GLFW_KEY_U: return ImGuiKey_U;
        case GLFW_KEY_V: return ImGuiKey_V;
        case GLFW_KEY_W: return ImGuiKey_W;
        case GLFW_KEY_X: return ImGuiKey_X;
        case GLFW_KEY_Y: return ImGuiKey_Y;
        case GLFW_KEY_Z: return ImGuiKey_Z;
        case GLFW_KEY_F1: return ImGuiKey_F1;
        case GLFW_KEY_F2: return ImGuiKey_F2;
        case GLFW_KEY_F3: return ImGuiKey_F3;
        case GLFW_KEY_F4: return ImGuiKey_F4;
        case GLFW_KEY_F5: return ImGuiKey_F5;
        case GLFW_KEY_F6: return ImGuiKey_F6;
        case GLFW_KEY_F7: return ImGuiKey_F7;
        case GLFW_KEY_F8: return ImGuiKey_F8;
        case GLFW_KEY_F9: return ImGuiKey_F9;
        case GLFW_KEY_F10: return ImGuiKey_F10;
        case GLFW_KEY_F11: return ImGuiKey_F11;
        case GLFW_KEY_F12: return ImGuiKey_F12;
        default: return ImGuiKey_None;
    }
}