#pragma once

#include <list>
#include <functional>


struct Event
{
    enum class Type
    {
        Input
    };

    Type type;
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
        Input,
        Subsystem,
        CategorySize
    };

    void Publish(Category category, Event* event);
    void Subscribe(Category category, std::function<void(Event*)> callback);
    void ProcessEvents();

private:
    std::list<Event*> queues[CategorySize];
    std::list<std::function<void(Event*)>> handlers[CategorySize];
};