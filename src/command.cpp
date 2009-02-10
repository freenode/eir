#include "command.h"
#include "exceptions.h"

#include <paludis/util/instantiation_policy-impl.hh>
#include <cstring>

using namespace eir;

template class paludis::InstantiationPolicy<CommandRegistry, paludis::instantiation_method::SingletonTag>;

bool CommandRegistry::IrcStringCmp::operator() (std::string s1, std::string s2)
{
    std::string::iterator i1 = s1.begin(), e1 = s1.end(), i2 = s2.begin(), e2 = s2.end();
    int res = 0;
    while(true)
    {
        if(i1 == e1)
            return i2 != e2;
        if(i2 == e2)
            return false;
        res = *i1++ - *i2++;
        if(res != 0)
            return res < 0;
    }
}

void CommandRegistry::dispatch(const Message *m, bool fatal_errors /* = false */)
{
    Context ctx("Processing handlers for command " + m->command);

    _dispatch(_handlers.find(m->command), m, fatal_errors);
}


void CommandRegistry::_dispatch(HandlerMap::iterator it, const Message *m, bool fatal_errors)
{
    if (it != _handlers.end())
    {
        HandlerList & l = (*it).second;
        for ( HandlerList::iterator i2 = l.begin(), i2_e = l.end(); i2 != i2_e; ++i2)
        {
            if (i2->second.first & m->source.type)
            {
                try
                {
                    i2->second.second(m);
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
                    m->source.error("Unknown error processing message " + m->command + ": " + e.what());
                }
            }
        }
    }
}

CommandRegistry::id CommandRegistry::add_handler(std::string s, const CommandRegistry::handler & h)
{
    return add_handler(s, sourceinfo::Any, h);
}

CommandRegistry::id CommandRegistry::add_handler(std::string s, unsigned int type,
                                                 const CommandRegistry::handler & h)
{
    static uintptr_t next_id = 0;

    Context ctx("Registering new handler for command " + s);

    HandlerMap::iterator mi = _handlers.find(s);
    if (mi == _handlers.end())
    {
        mi = _handlers.insert(std::make_pair(s, HandlerList())).first;
    }
    HandlerList& l = (*mi).second;
    l.insert(make_pair(id(++next_id), make_pair(type, h)));
    return id(next_id);
}

void CommandRegistry::remove_handler(id h)
{
    HandlerMap::iterator mi = _handlers.begin(), me = _handlers.end();
    for ( ; mi != me; ++mi)
    {
        (*mi).second.erase(h);
    }
}

