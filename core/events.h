#pragma once

#include "event_queue.h"

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

#define EVENT_TYPE(eType) Event##eType##() \
    {type = Event::Type::##eType;}

struct EventInputViews: public Event
{
    EVENT_TYPE(InputViews);

    glm::vec4 xr_left_eye_fov,  xr_right_eye_fov;
    glm::vec4 xr_left_eye_quat, xr_right_eye_quat;
    glm::vec3 xr_left_eye_pos,  xr_right_eye_pos;
};

struct EventInputPoses: public Event
{
    EVENT_TYPE(InputPoses);

    glm::vec4 xr_left_grip_quat, xr_right_grip_quat;
    glm::vec3 xr_left_grip_pos,  xr_right_grip_pos;

    glm::vec4 xr_left_aim_quat, xr_right_aim_quat;
    glm::vec3 xr_left_aim_pos,  xr_right_aim_pos;
};

struct EventInputFloats: public Event
{
    EVENT_TYPE(InputFloats);

    float l_squeeze_value;
    float r_squeeze_value;

    float l_trigger_value;
    float r_trigger_value;

    float l_thumbstick_x;
    float r_thumbstick_x;

    float l_thumbstick_y;
    float r_thumbstick_y;
};

struct EventInputKeyboard: public Event
{
    EVENT_TYPE(InputKeyboard);

    enum Action
    {
        KeyPressed,
        KeyRepeated,
        KeyReleased
    };

    int key;
    Action action;
};