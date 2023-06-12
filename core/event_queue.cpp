#include "event_queue.h"

void EventQueue::Publish(
    EventQueue::Category category, Event* event)
{
    queues[category].push_back(event);
}

void EventQueue::Subscribe(
    EventQueue::Category category, std::function<void(Event*)> callback)
{
    handlers[category].push_back(callback);
}

void EventQueue::ProcessEvents()
{
    for (int i = 0; i < CategorySize; i++)
    {
        while(!queues[i].empty())
        {
            Event* event = queues[i].front();
            queues[i].pop_front();
            for (std::function<void(Event*)> handler: handlers[i])
            {
                handler(event);
            }

            delete event;
        }
    }
}