#include "event_queue.h"

#include "validation.h"

void EventQueue::Publish(
    EventQueue::Category category, Event* event)
{
    queues[category].push_back(event);
}

int EventQueue::Subscribe(
    EventQueue::Category category, std::function<void(Event*)> callback)
{
    int handle = handleCount++;
    subscribers[category][handle] = callback;
    return handle;
}

void EventQueue::Unsubscribe(int handle)
{
    for (int i = 0; i < CategorySize; i++)
    {
        auto& it = subscribers[i].find(handle);
        if (it != subscribers[i].cend())
        {
            subscribers[i].erase(it);
            return;
        }
    }

    throw;
}

void EventQueue::ProcessEvents()
{
    for (int i = 0; i < CategorySize; i++)
    {
        // Capture the size of the queue before processing it.
        // If more events are added to the current queue 
        // while processing the queue events,
        // the additional events are NOT processed as current frame.
        int queueSize = queues[i].size();
        while(queueSize-- > 0)
        {
            Event* event = queues[i].front();
            queues[i].pop_front();
            for (auto& subscriber: subscribers[i])
            {
                subscriber.second(event);
            }

            delete event;
        }
    }
}