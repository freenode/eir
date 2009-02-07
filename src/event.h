#ifndef event_h
#define event_h

#include <tr1/functional>
#include <ctime>

namespace eir
{
    class EventManager
    {
        public:
            typedef std::tr1::function<void ()> event_func;
            typedef unsigned int id;

            virtual id add_event(time_t t, event_func f) = 0;
            virtual id add_recurring_event(time_t interval, event_func f) = 0;

            virtual void remove_event(id) = 0;

            static EventManager *get_instance();
    };
}

#endif
