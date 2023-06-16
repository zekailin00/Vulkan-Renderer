#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <functional>

#include "events.h"

enum class KeyCode
{

    /* GLFW keyboard inputs */
    KEY_UNKNOWN    = -1,
    KEY_SPACE      = 32,
    KEY_APOSTROPHE = 39, /* ' */
    KEY_COMMA      = 44, /* , */
    KEY_MINUS      = 45, /* - */
    KEY_PERIOD     = 46, /* . */
    KEY_SLASH      = 47, /* / */

    KEY_0 = 48,
    KEY_1 = 49,
    KEY_2 = 50,
    KEY_3 = 51,
    KEY_4 = 52,
    KEY_5 = 53,
    KEY_6 = 54,
    KEY_7 = 55,
    KEY_8 = 56,
    KEY_9 = 57,

    KEY_SEMICOLON = 59, /* ; */
    KEY_EQUAL     = 61, /* = */

    KEY_A = 65,
    KEY_B = 66,
    KEY_C = 67,
    KEY_D = 68,
    KEY_E = 69,
    KEY_F = 70,
    KEY_G = 71,
    KEY_H = 72,
    KEY_I = 73,
    KEY_J = 74,
    KEY_K = 75,
    KEY_L = 76,
    KEY_M = 77,
    KEY_N = 78,
    KEY_O = 79,
    KEY_P = 80,
    KEY_Q = 81,
    KEY_R = 82,
    KEY_S = 83,
    KEY_T = 84,
    KEY_U = 85,
    KEY_V = 86,
    KEY_W = 87,
    KEY_X = 88,
    KEY_Y = 89,
    KEY_Z = 90,

    KEY_LEFT_BRACKET  = 91 , /* [ */
    KEY_BACKSLASH     = 92 , /* \ */
    KEY_RIGHT_BRACKET = 93 , /* ] */
    KEY_GRAVE_ACCENT  = 96 , /* ` */
    KEY_WORLD_1       = 161, /* non-US #1 */
    KEY_WORLD_2       = 162, /* non-US #2 */

    KEY_ESCAPE    = 256,
    KEY_ENTER     = 257,
    KEY_TAB       = 258,
    KEY_BACKSPACE = 259,
    KEY_INSERT    = 260,
    KEY_DELETE    = 261,
    KEY_RIGHT     = 262,
    KEY_LEFT      = 263,
    KEY_DOWN      = 264,
    KEY_UP        = 265,
    KEY_PAGE_UP   = 266,
    KEY_PAGE_DOWN = 267,
    KEY_HOME      = 268,
    KEY_END       = 269,

    KEY_CAPS_LOCK    = 280,
    KEY_SCROLL_LOCK  = 281,
    KEY_NUM_LOCK     = 282,
    KEY_PRINT_SCREEN = 283,
    KEY_PAUSE        = 284,

    KEY_F1  = 290,
    KEY_F2  = 291,
    KEY_F3  = 292,
    KEY_F4  = 293,
    KEY_F5  = 294,
    KEY_F6  = 295,
    KEY_F7  = 296,
    KEY_F8  = 297,
    KEY_F9  = 298,
    KEY_F10 = 299,
    KEY_F11 = 300,
    KEY_F12 = 301,
    KEY_F13 = 302,
    KEY_F14 = 303,
    KEY_F15 = 304,
    KEY_F16 = 305,
    KEY_F17 = 306,
    KEY_F18 = 307,
    KEY_F19 = 308,
    KEY_F20 = 309,
    KEY_F21 = 310,
    KEY_F22 = 311,
    KEY_F23 = 312,
    KEY_F24 = 313,
    KEY_F25 = 314,

    KEY_KP_0 = 320,
    KEY_KP_1 = 321,
    KEY_KP_2 = 322,
    KEY_KP_3 = 323,
    KEY_KP_4 = 324,
    KEY_KP_5 = 325,
    KEY_KP_6 = 326,
    KEY_KP_7 = 327,
    KEY_KP_8 = 328,
    KEY_KP_9 = 329,

    KEY_KP_DECIMAL  = 330,
    KEY_KP_DIVIDE   = 331,
    KEY_KP_MULTIPLY = 332,
    KEY_KP_SUBTRACT = 333,
    KEY_KP_ADD      = 334,
    KEY_KP_ENTER    = 335,
    KEY_KP_EQUAL    = 336,

    KEY_LEFT_SHIFT    = 340,
    KEY_LEFT_CONTROL  = 341,
    KEY_LEFT_ALT      = 342,
    KEY_LEFT_SUPER    = 343,
    KEY_RIGHT_SHIFT   = 344,
    KEY_RIGHT_CONTROL = 345,
    KEY_RIGHT_ALT     = 346,
    KEY_RIGHT_SUPER   = 347,

    KEY_MENU = 348,

    /* OpenXR inputs */
    XR_LEFT_EYE_FOV = 500, XR_RIGHT_EYE_FOV,
    XR_LEFT_EYE_QUAT, XR_RIGHT_EYE_QUAT,
    XR_LEFT_EYE_POS,  XR_RIGHT_EYE_POS,

    XR_LEFT_GRIP_QUAT, XR_RIGHT_GRIP_QUAT,
    XR_LEFT_GRIP_POS,  XR_RIGHT_GRIP_POS,

    XR_LEFT_AIM_QUAT, XR_RIGHT_AIM_QUAT,
    XR_LEFT_AIM_POS,  XR_RIGHT_AIM_POS,

    XR_LEFT_SQEEZE, XR_RIGHT_SQEEZE,
    XR_LEFT_TRIGGER, XR_RIGHT_TRIGGER,

    XR_LEFT_THUMBSTICK_X, XR_RIGHT_THUMBSTICK_X,
    XR_LEFT_THUMBSTICK_Y, XR_RIGHT_THUMBSTICK_Y
};

class Input
{

public:
    static Input* GetInstance();

    glm::vec4 xr_left_eye_fov,  xr_right_eye_fov;
    glm::vec4 xr_left_eye_quat, xr_right_eye_quat;
    glm::vec3 xr_left_eye_pos,  xr_right_eye_pos;

    glm::vec4 xr_left_grip_quat, xr_right_grip_quat;
    glm::vec3 xr_left_grip_pos,  xr_right_grip_pos;

    glm::vec4 xr_left_aim_quat, xr_right_aim_quat;
    glm::vec3 xr_left_aim_pos,  xr_right_aim_pos;

    float l_squeeze_value, r_squeeze_value;
    float l_trigger_value, r_trigger_value;

    float l_thumbstick_x, r_thumbstick_x;
    float l_thumbstick_y, r_thumbstick_y;

    std::function<void(const char**)> glfwGetClipboard = nullptr;
    std::function<void(const char*)> glfwSetClipboard = nullptr;

    void SetGlfwGetClipboard(std::function<void(const char**)> fn)
    {
        glfwGetClipboard = fn;
    }

    void SetGlfwSetClipboard(std::function<void(const char*)> fn)
    {
        glfwSetClipboard = fn;
    }

    void SetClipboardText(const char* text)
    {
        if (glfwSetClipboard == nullptr)
            return;
        
        glfwSetClipboard(text);
    }

    void GetClipboardText(const char** text)
    {
        if (glfwGetClipboard == nullptr)
            return;
        
        glfwGetClipboard(text);
    }

private:
    static void AddXRViews(Event* event);
    static void AddXRPoses(Event* event);
    static void AddXRFloats(Event* event);
};