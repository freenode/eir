#include "eir.h"
#include "handler.h"

using namespace eir;

struct echo : CommandHandlerBase<echo>, Module
{
    void do_echo(const Message *m)
    {
        m->source.reply(m->args.at(0));
    }

    CommandHolder _id;

    echo() { _id = add_handler(filter_command_type("echo", sourceinfo::IrcCommand), &echo::do_echo); }
};

MODULE_CLASS(echo)

