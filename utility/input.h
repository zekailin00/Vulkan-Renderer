#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class GlfwWindow;
class OpenxrPlatform;
class OpenxrSession;
class VulkanRenderer;
class RenderTechnique;

class Input
{

public:
    enum class Key
    {
        XR_LEFT_EYE_FOV,  XR_RIGHT_EYE_FOV,
        XR_LEFT_EYE_QUAT, XR_RIGHT_EYE_QUAT,
        XR_LEFT_EYE_POS,  XR_RIGHT_EYE_POS,

        XR_LEFT_GRIP_QUAT, XR_RIGHT_GRIP_QUAT,
        XR_LEFT_GRIP_POS,  XR_RIGHT_GRIP_POS,

        XR_LEFT_AIM_QUAT, XR_RIGHT_AIM_QUAT,
        XR_LEFT_AIM_POS,  XR_RIGHT_AIM_POS,


        KEY_A, KEY_B, KEY_C, KEY_D, KEY_E,
        KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
        KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,
        KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
        KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

        KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,

        KEY_SPACE
    };

    static Input* GetInstance()
    {
        static Input input;
        return &input;
    }

    bool GetInputBool(Key key);
    float GetInputFloat(Key key);
    glm::vec2 GetInputFloat2(Key key);
    glm::vec3 GetInputFloat3(Key key);
    glm::vec4 GetInputFloat4(Key key);


    // Acquire inputs from two classes below
public:
    glm::vec4 xr_left_eye_fov,  xr_right_eye_fov;
    glm::vec4 xr_left_eye_quat, xr_right_eye_quat;
    glm::vec3 xr_left_eye_pos,  xr_right_eye_pos;

    glm::vec4 xr_left_grip_quat, xr_right_grip_quat;
    glm::vec3 xr_left_grip_pos,  xr_right_grip_pos;

    glm::vec4 xr_left_aim_quat, xr_right_aim_quat;
    glm::vec3 xr_left_aim_pos,  xr_right_aim_pos;
};