#include "eir.h"

#include "handler.h"

#include <paludis/util/tokeniser.hh>

using namespace std::tr1::placeholders;
using namespace eir;

struct JoinChannels : CommandHandlerBase<JoinChannels>, Module
{
    std::list<std::string> bot_channels;

    void add_channel(const Message *m)
    {
        bot_channels.push_back(m->args[0]);
        if (m->bot && m->bot->connected())
            m->bot->send("JOIN " + m->args[0]);

        m->source.reply("Added channel " + m->args[0]);
    }

    void remove_channel(const Message *m)
    {
        for (std::list<std::string>::iterator it = bot_channels.begin();
                it != bot_channels.end(); ++it)
        {
            if (*it == m->args[0])
                bot_channels.erase(it++);
        }

        if (m->bot && m->bot->connected())
            m->bot->send("PART " + m->args[0]);

        m->source.reply("Removed channel " + m->args[0]);
    }

    void on_connect(const Message *m)
    {
        for (std::list<std::string>::iterator it = bot_channels.begin();
                it != bot_channels.end(); ++it)
        {
            m->bot->send("JOIN " + *it);
        }
    }

    CommandHolder addch_id, join_id, rmch_id, part_id, conn_id;

    JoinChannels()
    {
        addch_id = add_handler(filter_command("channel").source_type(sourceinfo::ConfigFile),
                                &JoinChannels::add_channel);
        join_id = add_handler(filter_command_type("join", sourceinfo::IrcCommand).requires_privilege("admin"),
                                &JoinChannels::add_channel);
        part_id = add_handler(filter_command_type("part", sourceinfo::IrcCommand).requires_privilege("admin"),
                                &JoinChannels::remove_channel);
        conn_id = add_handler(filter_command("001").source_type(sourceinfo::RawIrc),
                                &JoinChannels::on_connect);
    }
};

MODULE_CLASS(JoinChannels)


