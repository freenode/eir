#include "command.h"
#include "exceptions.h"

#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <cstring>
#include <map>

using namespace eir;
using namespace paludis;

template class paludis::InstantiationPolicy<CommandRegistry, paludis::instantiation_method::SingletonTag>;

namespace paludis
{
    template <>
    struct Implementation<CommandRegistry>
    {
        typedef std::map<CommandRegistry::id, std::pair<Filter, CommandRegistry::handler> > HandlerMap;
        HandlerMap _handlers;
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
    for ( Implementation<CommandRegistry>::HandlerMap::iterator it = _imp->_handlers.begin();
            it != _imp->_handlers.end(); ++it)
    {
        if (it->second.first.match(m))
        {
            try
            {
                it->second.second(m);
            }
            catch (Exception &e)
            {
                if (e.fatal() || fatal_errors)
                    throw;
                m->source.error("Error processing message " + m->command + ": " +
                        e.message() + " (" + e.what() + ")");
            }
            catch (std::exception &e)
            {
                if (fatal_errors)
                    throw;
                m->source.error("Unknown error processing message " + m->command + ": " + e.what());
            }
        }
    }
}

CommandRegistry::id CommandRegistry::add_handler(Filter f, const CommandRegistry::handler & h)
{
    static uintptr_t next_id = 0;

    Context ctx("Registering new handler");

    _imp->_handlers.insert(std::make_pair(CommandRegistry::id(++next_id), std::make_pair(f, h)));
    return id(next_id);
}

void CommandRegistry::remove_handler(id h)
{
    _imp->_handlers.erase(h);
}

