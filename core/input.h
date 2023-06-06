#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "events.h"

enum class KeyCode
{
    KEY_A = 65,
    KEY_B, KEY_C, KEY_D, KEY_E, KEY_F,
    KEY_G, KEY_H, KEY_I, KEY_J, KEY_K,
    KEY_L, KEY_M, KEY_N, KEY_O, KEY_P,
    KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U,
    KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

    XR_LEFT_EYE_FOV = 500,  XR_RIGHT_EYE_FOV,
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

private:
    static void AddXRViews(Event* event);
    static void AddXRPoses(Event* event);
    static void AddXRFloats(Event* event);
};