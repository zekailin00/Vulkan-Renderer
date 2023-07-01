#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <functional>

#include "events.h"
#include "input_keycode.h"

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