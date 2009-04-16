#include "eir.h"

#include "handler.h"

using namespace std::tr1::placeholders;
using namespace eir;

struct Die : CommandHandlerBase<Die>, Module
{
    void die(const Message *m)
    {
        m->source.reply("Bye bye...");
        Logger::get_instance()->Log(m->bot, m->source.client, Logger::Admin, "DIE from " + m->source.raw);
        m->bot->disconnect("Shutting down (" + m->source.name + ")");
        throw DieException(m->source.client->nuh());
    }
    void restart(const Message *m)
    {
        m->source.reply("Restarting...");
        Logger::get_instance()->Log(m->bot, m->source.client, Logger::Admin, "RESTART from " + m->source.raw);
        m->bot->disconnect("Restarting (" + m->source.name + ")");
        throw RestartException();
    }

    CommandHolder die_id, restart_id;

    Die()
    {
        die_id = add_handler(filter_command_privilege("die", "admin"), &Die::die);
        restart_id = add_handler(filter_command_privilege("restart", "admin"), &Die::restart);
    }
};

MODULE_CLASS(Die)
