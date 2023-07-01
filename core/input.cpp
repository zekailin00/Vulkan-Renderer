#include "input.h"


Input* Input::GetInstance()
{
    static Input input;
    static bool initialized = false;

    if (!initialized)
    {
        EventQueue* queue = EventQueue::GetInstance();
        initialized = true;
    }

    return &input;
}

void Input::AddXRViews(Event* event)
{
    if (event->type == Event::Type::InputViews)
    {
        Input* input = Input::GetInstance();
        EventInputViews* e = dynamic_cast<EventInputViews*>(event);
    
        input->xr_left_eye_fov = e->xr_left_eye_fov;
        input->xr_right_eye_fov = e->xr_right_eye_fov;
        input->xr_left_eye_quat = e->xr_left_eye_quat;
        input->xr_right_eye_quat = e->xr_right_eye_quat;
        input->xr_left_eye_pos = e->xr_left_eye_pos;
        input->xr_right_eye_pos = e->xr_right_eye_pos;
    }
}

void Input::AddXRPoses(Event* event)
{
    throw;
}

void Input::AddXRFloats(Event* event)
{
    if (event->type == Event::Type::InputFloats)
    {
        Input* input = Input::GetInstance();
        EventInputFloats* e = dynamic_cast<EventInputFloats*>(event);
    
        input->l_squeeze_value = e->l_squeeze_value;
        input->r_squeeze_value = e->r_squeeze_value;

        input->l_trigger_value = e->l_trigger_value;
        input->r_trigger_value = e->r_trigger_value;

        input->l_thumbstick_x = e->l_thumbstick_x;
        input->r_thumbstick_x = e->r_thumbstick_x;

        input->l_thumbstick_y = e->l_thumbstick_y;
        input->r_thumbstick_y = e->r_thumbstick_y;
    }
}