#include "eir.h"
#include "handler.h"

using namespace eir;
using namespace std::tr1::placeholders;

struct Config : CommandHandlerBase<Config>, Module
{
    void config(const Message *m)
    {
        if (m->args.empty())
        {
            m->source.error("I need a setting name");
            return;
        }

        Bot::SettingsIterator it1 = m->bot->find_setting(m->args[0]);
        if (it1 != m->bot->end_settings())
        {
            m->source.reply(m->args[0] + " for " + m->bot->name() + " is " + (std::string)it1->second);
            return;
        }

        m->source.error("Can't find setting " + m->args[0]);
    }

    CommandHolder _id;

    Config() { _id = add_handler(filter_command_type("config", sourceinfo::IrcCommand), &Config::config); }
};

MODULE_CLASS(Config)


