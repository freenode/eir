#include "bot.h"
#include "command.h"

using namespace eir;
using namespace std::tr1::placeholders;
/*
struct echo {
    void do_echo(const Message *m)
    {
        if (m->destination != m->bot->nick())
            return;
        if (m->args[0].substr(0, 4) == "echo")
            m->bot->send("PRIVMSG " + m->source + " :" + m->args[0].substr(4));
    }
    echo() { _id = CommandRegistry::get_instance()->add_handler("PRIVMSG", std::tr1::bind(std::tr1::mem_fn(&echo::do_echo), this, _1)); }
    ~echo() { CommandRegistry::get_instance()->remove_handler(_id); }
    CommandRegistry::id _id;
};

echo e;
*/
