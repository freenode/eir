#include "command.h"

#include <cstring>

using namespace eir;

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

void CommandRegistry::dispatch(const Message *m)
{
    _dispatch(_handlers.find(m->command), m);
    _dispatch(_handlers.find(""), m);
}


void CommandRegistry::_dispatch(HandlerMap::iterator it, const Message *m)
{
    if (it != _handlers.end())
    {
        HandlerList & l = (*it).second;
        for ( HandlerList::iterator i2 = l.begin(), i2_e = l.end(); i2 != i2_e; ++i2)
        {
            (*i2).second(m);
        }
    }
}

CommandRegistry::id CommandRegistry::add_handler(std::string s, const CommandRegistry::handler & h)
{
    static uintptr_t next_id = 0;

    HandlerMap::iterator mi = _handlers.find(s);
    if (mi == _handlers.end())
    {
        mi = _handlers.insert(std::make_pair(s, HandlerList())).first;
    }
    HandlerList& l = (*mi).second;
    l.insert(make_pair(id(++next_id), h));
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

