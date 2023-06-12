#pragma once

#include <map>
#include <list>
#include <functional>


struct Event
{
    enum class Type
    {
        InputViews,
        InputPoses,
        InputFloats,

        Keyboard,
        KeyboardImgui,
        CharacterUTF32,
        MouseButton,
        MousePosition,
        MouseWheel,
        CursorEnter
    };

    Type type;

    virtual ~Event() = default;
};

/**
 * @brief Asynchronous, multicasting event queue.
 * Events are allocated by the publisher, but owned and freed by the queue.
 * Subscribers process one event at a time (NOT in batch).
 */
class EventQueue
{
public:
    enum Category
    {
        None = 0,
        InputXR,
        InputGFLW,
        Subsystem,
        CategorySize
    };

    static EventQueue* GetInstance()
    {
        static EventQueue queue;
        return &queue;
    }

    void Publish(Category category, Event* event);
    int Subscribe(Category category, std::function<void(Event*)> callback);
    void Unsubscribe(int handle);
    void ProcessEvents();

private:
    std::list<Event*> queues[CategorySize];
    std::map<int, std::function<void(Event*)>> subscribers[CategorySize];
    unsigned int handleCount = 0;
};