#pragma once

#include <list>
#include <functional>


struct Event
{
    enum class Type
    {
        InputViews,
        InputPoses,
        InputFloats,
        InputKeyboard
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
    void Subscribe(Category category, std::function<void(Event*)> callback);
    void ProcessEvents();

private:
    std::list<Event*> queues[CategorySize];
    std::list<std::function<void(Event*)>> handlers[CategorySize];
};