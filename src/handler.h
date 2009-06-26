#ifndef handler_h
#define handler_h

#include "command.h"
#include "event.h"
#include "logger.h"
#include "storage.h"
#include <tr1/functional>


namespace eir
{
    template <class T_>
    struct CommandHandlerBase
    {
        template <class F_>
        CommandRegistry::id add_handler(Filter f, F_ h)
        {
            return eir::CommandRegistry::get_instance()->add_handler(f,
                    std::tr1::bind(h, static_cast<T_*>(this), std::tr1::placeholders::_1));
        }

        template <class F_>
        EventManager::id add_event(time_t t, F_ h)
        {
            return EventManager::get_instance()->add_event(t,
                    std::tr1::bind(h, static_cast<T_*>(this)));
        }

        template <class F_>
        EventManager::id add_recurring_event(time_t t, F_ h)
        {
            return EventManager::get_instance()->add_recurring_event(t,
                    std::tr1::bind(h, static_cast<T_*>(this)));
        }
    };

    class CommandHolder :
        public paludis::InstantiationPolicy<CommandHolder, paludis::instantiation_method::NonCopyableTag>
    {
        private:
            CommandRegistry::id _id;

            void _release() { if (_id) CommandRegistry::get_instance()->remove_handler(_id); _id = 0; }

        public:
            CommandHolder() : _id(0)
            { }
            CommandHolder(CommandRegistry::id id) : _id(id)
            { }
            const CommandHolder & operator= (CommandRegistry::id id)
            { _release(); _id = id; return *this; }

            ~CommandHolder() { _release(); }
    };

    class EventHolder :
        public paludis::InstantiationPolicy<EventHolder, paludis::instantiation_method::NonCopyableTag>
    {
        private:
            EventManager::id _id;

            void _release() { if (_id) EventManager::get_instance()->remove_event(_id); _id = 0; }

        public:
            EventHolder() : _id(0)
            { }
            EventHolder(EventManager::id id) : _id(id)
            { }
            const EventHolder & operator= (EventManager::id id)
            { _release(); _id = id; return *this; }

            ~EventHolder() { _release(); }
    };

    class LogBackendHolder :
        public paludis::InstantiationPolicy<LogBackendHolder, paludis::instantiation_method::NonCopyableTag>
    {
        private:
            Logger::BackendId _id;

            void _release() { if (_id) Logger::get_instance()->unregister_backend(_id); _id = 0; }

        public:
            LogBackendHolder() : _id(0)
            { }
            LogBackendHolder(Logger::BackendId id) : _id(id)
            { }
            const LogBackendHolder & operator= (Logger::BackendId id)
            { _release(); _id = id; return *this; }

            ~LogBackendHolder() { _release(); }
    };

    class StorageBackendHolder :
        public paludis::InstantiationPolicy<StorageBackendHolder, paludis::instantiation_method::NonCopyableTag>
    {
        private:
            StorageManager::BackendId _id;

            void _release() { if (_id) StorageManager::get_instance()->unregister_backend(_id); _id = 0; }

        public:
            StorageBackendHolder() : _id(0)
            { }
            StorageBackendHolder(StorageManager::BackendId id) : _id(id)
            { }
            const StorageBackendHolder & operator= (StorageManager::BackendId id)
            { _release(); _id = id; return *this; }

            ~StorageBackendHolder() { _release(); }
    };
}

#endif
