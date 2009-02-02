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

    CommandRegistry::id _id;

    echo() { _id = add_handler("echo", sourceinfo::IrcCommand, &echo::do_echo); }
    ~echo() { remove_handler(_id); }
};

echo e;

