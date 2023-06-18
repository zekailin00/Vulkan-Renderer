#pragma once

#include "event_queue.h"
#include "component.h"

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>


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

struct EventKeyboard: public Event
{
    EVENT_TYPE(Keyboard);

    enum Action
    {
        KeyReleased = 0,
        KeyPressed = 1,
        KeyRepeated = 2
    };

    int keyCode;
    Action action;
};

struct EventKeyboardImgui: public Event
{
    EVENT_TYPE(KeyboardImgui);

    int keyCode;
    bool pressed;
};

struct EventCharacterUTF32: public Event
{
    EVENT_TYPE(CharacterUTF32);

    unsigned int c;
};

struct EventMouseButton: public Event
{
    EVENT_TYPE(MouseButton);

    enum Button
    {
        ButtonLeft = 1,
        ButtonRight = 2,
        ButtonMiddle = 3
    };

    enum Action
    {
        ButtonRelease = 0,
        ButtonPress = 1
    };

    Button button;
    Action action;
};

struct EventMousePosition: public Event
{
    EVENT_TYPE(MousePosition);

    glm::vec2 pos;
};

struct EventMouseWheel: public Event
{
    EVENT_TYPE(MouseWheel);

    glm::vec2 offset;
};

struct EventCursorEnter: public Event
{
    EVENT_TYPE(CursorEnter);

    int entered;
};


class Entity;

struct EventEntitySelected: public Event
{
    EVENT_TYPE(EntitySelected);

    Entity* entity;
};

struct EventNewEntity: public Event
{
    EVENT_TYPE(NewEntity);

    Entity* parent;
};

struct EventDeleteEntity: public Event
{
    EVENT_TYPE(DeleteEntity);

    Entity* entity;
};

struct EventDeleteComponent: public Event
{
    EVENT_TYPE(DeleteComponent);

    Entity* entity;
    Component::Type componentType;
};

struct EventMaterialSelected: public Event
{
    EVENT_TYPE(MaterialSelected);

    void* materialPtr;
};

struct EventTextureSelected: public Event
{
    EVENT_TYPE(TextureSelected);

    void* texturePtr;
};

struct EventProjectOpen: public Event
{
    EVENT_TYPE(ProjectOpen);

    void* assetManager;
};

struct EventCloseProject: public Event
{
    EVENT_TYPE(CloseProject);

    void* assetManager;
};

struct EventSceneOpen: public Event
{
    EVENT_TYPE(SceneOpen);

    void* scene;
};

struct EventCloseScene: public Event
{
    EVENT_TYPE(CloseScene);

    bool saveToFs;
};

struct EventSceneSelected: public Event
{
    EVENT_TYPE(SceneSelected);

    void* scene;
};

struct EventWorkspaceChanged: public Event
{
    EVENT_TYPE(WorkspaceChanged);
};

struct EventSaveProject: public Event
{
    EVENT_TYPE(SaveProject);
};

struct EventSimStartVR: public Event
{
    EVENT_TYPE(SimStartVR);
};

struct EventSimStart: public Event
{
    EVENT_TYPE(SimStart);
};

struct EventSimStop: public Event
{
    EVENT_TYPE(SimStop);
};