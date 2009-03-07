#include "eir.h"

#include <functional>
#include <iostream>

using namespace eir;
using namespace std::tr1::placeholders;

struct Whoami : public CommandHandlerBase<Whoami>
{
    void whoami(const Message *m)
    {
        if (!m->source.client)
        {
            m->source.reply("I don't know who you are.");
            return;
        }

        m->source.reply("You are " + m->source.client->nick());

        std::string privbuf;
        for(PrivilegeSet::iterator it = m->source.client->privs().begin();
                it != m->source.client->privs().end(); ++it)
            privbuf += *it + " ";

        if (!privbuf.empty())
            m->source.reply("You have privileges " + privbuf);
    }

    CommandHolder _id;

    Whoami()
    {
        _id = add_handler(filter_command_type("whoami", sourceinfo::IrcCommand), &Whoami::whoami);
    }
} whoami;



 
