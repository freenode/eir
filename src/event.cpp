#include "event_internal.h"

using namespace eir;

EventManager *EventManager::get_instance()
{
    static EventManagerImpl _instance;
    return &_instance;
}

static EventManager::id next_id = 0;

EventManager::id EventManagerImpl::add_event(time_t t, EventManager::event_func f)
{
    event::ptr e(new event(next_id++, t, 0, f));
    events.push_back(e);
    return next_id;
}

EventManager::id EventManagerImpl::add_recurring_event(time_t i, EventManager::event_func f)
{
    event::ptr e(new event(next_id++, time(NULL) + i, i, f));
    events.push_back(e);
    return next_id;
}

void EventManagerImpl::remove_event(EventManager::id id)
{
    for (event_list::iterator it = events.begin(); it != events.end(); ++it)
    {
        if ((*it)->_id ==  id)
            events.erase(it++);
    }
}

time_t EventManagerImpl::next_event_time() const
{
    time_t t = 0;
    for (event_list::const_iterator it = events.begin(); it != events.end(); ++it)
        if (t == 0 || (*it)->next_time < t)
            t = (*it)->next_time;
    return t;
}

void EventManagerImpl::run_events()
{
    time_t current_time = time(NULL);
    for (event_list::iterator it = events.begin(); it != events.end(); )
    {
        if ((*it)->next_time <= current_time)
        {
            (*it)->func();

            if ((*it)->interval)
                (*it)->next_time += (*it)->interval;
            else
            {
                events.erase(it++);
                continue;
            }
        }
        it++;
    }
}

