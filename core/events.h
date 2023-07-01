#pragma once

#include "event_queue.h"
#include "component.h"
#include "input_keycode.h"

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>


#define EVENT_TYPE(eType) Event##eType() \
    {type = Event::Type::eType;}

struct EventInputViews: public Event
{
    EVENT_TYPE(InputViews);

    glm::vec4 xr_left_eye_fov,  xr_right_eye_fov;
    glm::vec4 xr_left_eye_quat, xr_right_eye_quat;
    glm::vec3 xr_left_eye_pos,  xr_right_eye_pos;
};

struct EventLeftAimPose: public Event
{
    EVENT_TYPE(LeftAimPose);

    glm::mat4 transform;
};

struct EventRightAimPose: public Event
{
    EVENT_TYPE(RightAimPose);

    glm::mat4 transform;
};

struct EventLeftGripPose: public Event
{
    EVENT_TYPE(LeftGripPose);

    glm::mat4 transform;
};

struct EventRightGripPose: public Event
{
    EVENT_TYPE(RightGripPose);

    glm::mat4 transform;
};

struct EventControllerInput: public Event
{
    EVENT_TYPE(ControllerInput);

    /**
     * Depending on the keycode,
     * either float or boolean value is valid
     */

    KeyCode keycode;
    float floatValue;
    bool boolValue;
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