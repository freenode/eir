#include "eir.h"
#include "handler.h"

using namespace eir;
using namespace std::tr1::placeholders;

struct echo : public CommandHandlerBase<echo>
{
    void do_echo(const Message *m)
    {
        m->source.reply(m->args.at(0));
    }

    CommandHolder _id;

    echo() { _id = add_handler(filter_command_type("echo", sourceinfo::IrcCommand), &echo::do_echo); }
};

echo e;

