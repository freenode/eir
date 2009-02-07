#include "event.h"
#include <list>
#include <tr1/memory>

namespace eir
{
    class EventManagerImpl : public EventManager
    {
        public:
            virtual id add_event(time_t t, event_func f);
            virtual id add_recurring_event(time_t interval, event_func f);

            virtual void remove_event(id);

            time_t next_event_time() const;
            void run_events();

        private:
            struct event {
                id _id;
                time_t next_time;
                time_t interval;
                event_func func;
                event(id i, time_t t, time_t in, event_func f)
                    : _id(i), next_time(t), interval(in), func(f)
                { }
                typedef std::tr1::shared_ptr<event> ptr;
            };
            typedef std::list<event::ptr> event_list;
            event_list events;
    };
}
