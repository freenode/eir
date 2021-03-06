#include "eir.h"
#include "handler.h"
#include "help.h"

#include <paludis/util/tokeniser.hh>
#include <paludis/util/join.hh>

#include <algorithm>

using namespace eir;

namespace
{
    const char *help_help = "help <topic|command> -- Gives help on a given topic or command.";
}

struct helper : CommandHandlerBase<helper>, Module
{
    void do_help(const Message *m, std::string topic)
    {
        Value& help_root = GlobalSettingsManager::get_instance()->get("help_root");
        if (help_root.Type() != Value::kvarray)
        {
            m->source.error("No help has been defined");
            return;
        }

        Value& help_topic = help_root[topic];
        if (help_topic.Type() != Value::kvarray ||
             ( !!help_topic["priv"] &&
               (!m->source.client || !m->source.client->privs().has_privilege(help_topic["priv"]))))
        {
            m->source.error("No help for '" + topic + "'.");
            return;
        }

        if (!m->source.reply_func)
            return;

        std::list<std::string> reply;
        paludis::tokenise<paludis::delim_kind::AnyOfTag, paludis::delim_mode::DelimiterTag>
                    (help_topic["text"], "\r\n", "", std::back_inserter(reply));
        std::for_each(reply.begin(), reply.end(), m->source.reply_func);
    }

    void do_help_index(const Message *m)
    {
        std::list<std::string> topics;
        Value& help_index = GlobalSettingsManager::get_instance()->get("help_index");
        for (KeyValueArray::iterator it = help_index.KV().begin(); it != help_index.KV().end(); ++it)
        {
            if (it->second.String().empty() ||
                 (m->source.client && m->source.client->privs().has_privilege(it->second)))
            {
                topics.push_back(it->first);
            }
        }
        m->source.reply("Available help topics are: " + paludis::join(topics.begin(), topics.end(), " "));
    }

    void help(const Message *m)
    {
        if (m->args.empty())
            do_help_index(m);
        else
            do_help(m, m->args[0]);
    }

    CommandHolder _id;
    HelpTopicHolder helptopic;
    HelpIndexHolder index;

    helper() : helptopic("help", "", help_help), index("help", "")
    { _id = add_handler(filter_command_type("help", sourceinfo::IrcCommand), &helper::help); }
};

MODULE_CLASS(helper)


