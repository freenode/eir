#include "command.h"
#include "exceptions.h"
#include "logger.h"

#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <cstring>
#include <map>
#include <stdint.h>

using namespace eir;
using namespace paludis;

template class paludis::InstantiationPolicy<CommandRegistry, paludis::instantiation_method::SingletonTag>;

namespace
{
    struct HandlerMapEntry {
        Filter filter;
        CommandRegistry::handler handler;
        bool quiet;
        HandlerMapEntry(Filter f, CommandRegistry::handler h, bool q)
            : filter(f), handler(h), quiet(q)
        { }
    };
}

namespace paludis
{
    template <>
    struct Implementation<CommandRegistry>
    {
        typedef std::map<CommandRegistry::id, HandlerMapEntry> HandlerMap;
        HandlerMap _handlers[3];
    };
}

CommandRegistry::CommandRegistry()
    : paludis::PrivateImplementationPattern<CommandRegistry>(new Implementation<CommandRegistry>)
{
}

CommandRegistry::~CommandRegistry()
{
}

void CommandRegistry::dispatch(const Message *m, bool fatal_errors)
{
    for (int i=0; i < 3; ++i)
    {
        for ( Implementation<CommandRegistry>::HandlerMap::iterator it = _imp->_handlers[i].begin();
                it != _imp->_handlers[i].end(); ++it)
        {
            if (it->second.filter.match(m))
            {
                try
                {
                    it->second.handler(m);
                }
                catch (Exception &e)
                {
                    if (e.fatal() || fatal_errors)
                        throw;

                    if (!(it->second.quiet))
                        m->source.error("I have suffered a terrible failure. (" + e.message() + ") (" + e.what() + ")");

                    Logger::get_instance()->Log(m->bot, m->source.client, Logger::Warning,
                            "Error processing message " + m->command + ": " + e.message() + " (" + e.what() + ")");
                }
                catch (std::exception &e)
                {
                    if (fatal_errors)
                        throw;
                    m->source.error(std::string("I have suffered a terrible failure. (") + e.what() + ")");
                    Logger::get_instance()->Log(m->bot, m->source.client, Logger::Warning,
                            "Unknown error processing message " + m->command + ": " + e.what());
                }
            }
        }
    }
}

CommandRegistry::id CommandRegistry::add_handler(Filter f, const CommandRegistry::handler & h, bool quiet_errors, Message::Order order)
{
    static uintptr_t next_id = 1;

    Context ctx("Registering new handler");

    _imp->_handlers[order].insert(std::make_pair(CommandRegistry::id(++next_id),
                                    HandlerMapEntry(f, h, quiet_errors)));
    return id(next_id);
}

void CommandRegistry::remove_handler(id h)
{
    for (int i=0; i < 3; ++i)
        _imp->_handlers[i].erase(h);
}

