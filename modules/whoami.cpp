#include "eir.h"

#include <functional>
#include <iostream>

using namespace eir;
using namespace std::tr1::placeholders;

struct Whoami : public CommandHandlerBase<Whoami>
{
    CommandRegistry::id _id;

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

    Whoami()
    {
        _id = add_handler("whoami", sourceinfo::IrcCommand, &Whoami::whoami);
    }
    ~Whoami()
    {
        remove_handler(_id);
    }
} whoami;



 
