#include "logger.h"

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>

using namespace eir;
using namespace paludis;

#include <list>

template class paludis::InstantiationPolicy<Logger, paludis::instantiation_method::SingletonTag>;

namespace
{
    struct LogDestinationInfo
    {
        Logger::BackendId backend;
        Logger::DestinationId id;
        LogDestination *dest;
        Logger::Type typemask;
        LogDestinationInfo(Logger::BackendId b, Logger::DestinationId i, LogDestination *d, Logger::Type t)
            : backend(b), id(i), dest(d), typemask(t)
        { }
    };

    struct LogBackendInfo
    {
        Logger::BackendId id;
        std::string name;
        LogBackend *backend;
        LogBackendInfo(Logger::BackendId i, std::string n, LogBackend *b)
            : id(i), name(n), backend(b)
        { }
    };
}

namespace paludis
{
    template <>
    struct Implementation<Logger>
    {
        std::list<LogDestinationInfo> destinations;
        std::list<LogBackendInfo> backends;
    };
}

Logger::BackendId Logger::register_backend(std::string name, LogBackend *b)
{
    static unsigned int next_id = 0;
    _imp->backends.push_back(LogBackendInfo(++next_id, name, b));
    return next_id;
}

void Logger::unregister_backend(BackendId id)
{
    std::list<LogDestinationInfo>::iterator it = _imp->destinations.begin();

    while (it != _imp->destinations.end())
    {
        if (it->backend == id)
        {
            delete it->dest;
            _imp->destinations.erase(it++);
        }
        else
            ++it;
    }

    std::list<LogBackendInfo>::iterator it2 = _imp->backends.begin();

    while (it2 != _imp->backends.end())
    {
        if (it2->id == id)
        {
            delete it2->backend;
            _imp->backends.erase(it2++);
        }
        else
            ++it;
    }
}

Logger::DestinationId Logger::add_destination(std::string type, std::string arg, Type types)
{
    std::list<LogBackendInfo>::iterator backend = _imp->backends.begin(); 

    for ( ; backend != _imp->backends.end(); ++backend)
    {
        if (backend->name == type)
            break;
    }

    if (backend == _imp->backends.end())
        throw ConfigurationError("No such log type " + type);

    static DestinationId next_id = 0;

    LogDestination *d = backend->backend->create_destination(arg);

    _imp->destinations.push_back(LogDestinationInfo(backend->id, ++next_id, d, types));

    return next_id;
}

void Logger::remove_destination(DestinationId id)
{
    std::list<LogDestinationInfo>::iterator it = _imp->destinations.begin();

    while (it != _imp->destinations.end())
    {
        if (it->id == id)
            _imp->destinations.erase(it++);
        else
            ++it;
    }
}

void Logger::Log(Bot *bot, Client *source, Type type, std::string text)
{
    for (std::list<LogDestinationInfo>::iterator it = _imp->destinations.begin();
            it != _imp->destinations.end(); ++it)
    {
        if (it->typemask & type)
        {
            it->dest->Log(bot, source, text);
        }
    }
}

void Logger::Log(Bot *b, std::shared_ptr<Client> s, Type t, std::string text)
{
    Log(b, s.get(), t, text);
}

void Logger::clear_logs()
{
    for (std::list<LogDestinationInfo>::iterator it = _imp->destinations.begin();
            it != _imp->destinations.end(); it = _imp->destinations.erase(it))
    {
        delete it->dest;
    }
}

Logger::Logger()
    : PrivateImplementationPattern<Logger>(new Implementation<Logger>)
{
}

Logger::~Logger()
{
}

#include "handler.h"

namespace
{
    Logger::Type TypeFromString(std::string s)
    {
        if (s == "debug")
            return Logger::Debug;
        if (s == "command")
            return Logger::Command;
        if (s == "info")
            return Logger::Info;
        if (s == "privs")
            return Logger::Privs;
        if (s == "warning")
            return Logger::Warning;
        if (s == "raw")
            return Logger::Raw;
        if (s == "admin")
            return Logger::Admin;
        return 0;
    }
    struct LogCreator : public CommandHandlerBase<LogCreator>
    {
        CommandHolder add_log_id, clear_log_id;

        void add_log(const Message *m)
        {
            std::vector<std::string>::const_iterator it = m->args.begin();
            std::string type = *it++;
            std::string arg = *it++;

            Logger::Type types(0);

            for ( ; it != m->args.end(); ++it)
            {
                types |= TypeFromString(*it);
            }

            Logger::get_instance()->add_destination(type, arg, types);
        }

        void clear_logs(const Message *)
        {
            Logger::get_instance()->clear_logs();
        }

        LogCreator()
        {
            add_log_id = add_handler(filter_command("log").requires_privilege("admin").or_config(),
                                     &LogCreator::add_log);
            clear_log_id = add_handler(filter_command_type("clear_lists", sourceinfo::Internal),
                                        &LogCreator::clear_logs);
        }
    };

    LogCreator lc;
}
