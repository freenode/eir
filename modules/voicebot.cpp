#include "eir.h"

using namespace eir;
using namespace std::tr1::placeholders;

#include <list>
#include "match.h"

#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>

#include "times.h"

#include <algorithm>

#include "help.h"

namespace
{
    const std::string default_time_fmt("%F %T");
    const std::string default_expiry("1d");

    std::string format_time(Bot *b, time_t t)
    {
        if (t == 0)
            return "never";

        char datebuf[128];
        tm time;
        localtime_r(&t, &time);
        strftime(datebuf,
                 sizeof(datebuf),
                 b->get_setting_with_default("voice_time_format", default_time_fmt).c_str(),
                 &time);
        return std::string(datebuf);
    }

    time_t get_default_expiry(Bot *b)
    {
        std::string time = b->get_setting_with_default("default_voice_expiry", default_expiry);
        return parse_time(time);
    }

    const char *help_voicebot =
        "The voicebot module exists to manage a moderated channel. It maintains a list of hostmask entries that "
        "will not be voiced (the \002DNV\002 list), and voices, on request, all unvoiced users in the channel that "
        "do not match any of those entries.\n"
        "Available voicebot commands are: voice check match add remove edit.\n"
        "Relevant settings are \037voicebot_channel\037, \037voicebot_admin_channel\037, \037default_voice_expiry\037, "
        "and \037voice_time_format\037.";
    const char *help_voice =
        "\002voice\002. Voices all users who are in \037voicebot_channel\037 and do not match a DNV list entry.";
    const char *help_check =
        "\002check\002. Displays a list of users in \037voicebot_channel\037 who would be voiced by the \002voice\002 "
        "command, and a list of those unvoiced users who would not be voiced.";
    const char *help_match =
        "\002match <mask>|<nick>\002. Displays the DNV list entries that match the given argument. If the nickname "
        "of a currently visible user is given, entries matching that user are shown. Otherwise, it is interpreted as "
        "a hostmask.";
    const char *help_add =
        "\002add <mask> [time] <reason>\002. Adds a DNV entry <mask>, expiring in <time>, with comment <reason>.\n"
        "If \037time\037 is not specified, the default_voice_expiry setting is used. \037time\037 may "
        "be zero, in which case the entry will not expire. \037reason\037 may be empty.";
    const char *help_remove =
        "\002remove <mask>\002. Removes all entries from the DNV list that match the given mask.\n"
        "Note that 'remove *' will clear the list.";
    const char *help_edit =
        "\002edit <mask> [time] [reason]\002. Edits the expiry time and/or comment of an existing DNV entry. If "
        "\037time\037 is given but \037reason\037 is not, then only the expiry will be changed. Similarly if "
        "\037reason\037 is given but \037time\037 is not, then the expiry will be left unchanged.";


    Value voiceentry(std::string bot, std::string mask, std::string setter, std::string reason,
                     time_t set, time_t expires)
    {
        Value v(Value::kvarray);
        v["bot"] = bot;
        v["mask"] = mask;
        v["setter"] = setter;
        v["reason"] = reason;
        v["set"] = set;
        v["expires"] = expires;
        return v;
    }

    struct Removed
    {
        bool operator() (const Value& v)
        {
            return v.Type() == Value::kvarray && v.KV().find("removed") != v.KV().end();
        }
    };

    void do_removals(Value& list)
    {
        Value newlist(Value::array);
        std::remove_copy_if(list.begin(), list.end(), std::back_inserter(newlist.Array()), Removed());
        std::swap(list, newlist);
    }
}

struct voicebot : CommandHandlerBase<voicebot>, Module
{
    Value &dnv, &old;

    void do_add(const Message *m)
    {
        if (m->args.empty())
        {
            m->source.error("Need at least one argument");
            return;
        }

        time_t expires;

        std::vector<std::string>::const_iterator it = m->args.begin();
        std::string mask = *it++;

        if ((*it)[0] == '~')
            expires = parse_time(*it++);
        else
            expires = get_default_expiry(m->bot);

        if (expires != 0)
            expires += time(NULL);

        std::string reason = paludis::join(it, m->args.end(), " ");

        // If this looks like a plain nick instead of a mask, treat it as a nickname
        // mask.
        if (mask.find_first_of("!@*") == std::string::npos)
            mask += "!*@*";

        for (ValueArray::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            if (mask_match((*it)["mask"], mask))
            {
                m->source.reply("Mask already matched by " + (*it)["mask"]);
                return;
            }
        }

        dnv.push_back(voiceentry(m->bot->name(), mask, m->source.name, reason, time(NULL), expires));
        m->source.reply("Added " + mask);

        Logger::get_instance()->Log(m->bot, m->source.client, Logger::Command, "ADD " + mask);
    }

    void do_change(const Message *m)
    {
        if (m->args.empty())
        {
            m->source.error("Need at least one argument");
            return;
        }

        time_t expires = 0;

        std::vector<std::string>::const_iterator it = m->args.begin();
        std::string mask = *it++;

        if ((*it)[0] == '~')
            expires = parse_time(*it++);
        if (expires > 0)
            expires += time(NULL);

        std::string reason = paludis::join(it, m->args.end(), " ");

        // If this looks like a plain nick instead of a mask, treat it as a nickname
        // mask.
        if (mask.find_first_of("!@*") == std::string::npos)
            mask += "!*@*";

        bool found = false;

        for (ValueArray::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            if (mask_match(mask, (*it)["mask"]))
            {
                if (expires)
                    (*it)["expires"] = expires;
                if (!reason.empty())
                    (*it)["reason"] = reason;
                found = true;
                m->source.reply("Updated " + (*it)["mask"]);
            }
        }
        if (!found)
            m->source.reply("No entry matches " + mask);

        Logger::get_instance()->Log(m->bot, m->source.client, Logger::Command, "CHANGE " + mask);
    }

    void do_remove(const Message *m)
    {
        if (m->args.empty())
        {
            m->source.error("Need at least one argument");
            return;
        }

        std::string mask = m->args[0];

        if (mask.find_first_of("!@*") == std::string::npos)
            mask += "!*@*";

        for (ValueArray::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            if (mask_match(mask, (*it)["mask"]))
            {
                Bot *bot = BotManager::get_instance()->find((*it)["bot"]);
                m->source.reply("Removing " + (*it)["mask"] + " (" + (*it)["reason"] + ") " +
                        "(added by " + (*it)["setter"] + " on " + format_time(bot, (*it)["set"].Int()) + ")");

                old.push_back(*it);
                (*it)["removed"] = 1;
            }
        }

        do_removals(dnv);

        Logger::get_instance()->Log(m->bot, m->source.client, Logger::Command, "REMOVE " + mask);
    }

    void do_list(const Message *m)
    {
        for (ValueArray::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            Bot *bot = BotManager::get_instance()->find((*it)["bot"]);
            m->source.reply((*it)["mask"] + " (" + (*it)["reason"] + ") (added by " + 
                    (*it)["setter"] + " on " + format_time(bot, (*it)["set"].Int()) +
                    ", expires " + format_time(bot, (*it)["expires"].Int()) + ")");
        }

        m->source.reply("*** End of DNV list");
    }

    void build_voice_lists(Channel::ptr channel, 
                           std::list<std::string> *tovoice,
                           std::list<std::string> *tonotvoice)
    {
        for (Channel::MemberIterator it = channel->begin_members(); it != channel->end_members(); ++it)
        {
            if ((*it)->has_mode('v'))
                continue;

            bool matched = false;
            for (ValueArray::iterator i2 = dnv.begin(); i2 != dnv.end(); ++i2)
            {
                if (match((*i2)["mask"], (*it)->client->nuh()))
                {
                    matched = true;
                    break;
                }
            }

            if (matched)
                tonotvoice->push_back((*it)->client->nick());
            else
                tovoice->push_back((*it)->client->nick());
        }
    }

    void do_check(const Message *m)
    {
        std::string channelname = m->bot->get_setting("voicebot_channel");
        if (channelname.empty())
        {
            m->source.error("voicebot_channel not defined.");
            return;
        }

        Channel::ptr channel = m->bot->find_channel(channelname);
        if (!channel)
        {
            m->source.error("Couldn't find channel " + channelname);
            return;
        }

        std::list<std::string> tovoice, tonotvoice;

        build_voice_lists(channel, &tovoice, &tonotvoice);

        m->source.reply("Needing voice: " + paludis::join(tovoice.begin(), tovoice.end(), " "));
        m->source.reply("Not voicing: " + paludis::join(tonotvoice.begin(), tonotvoice.end(), " "));
    }

    void do_voice(const Message *m)
    {
        std::string channelname = m->bot->get_setting("voicebot_channel");
        if (channelname.empty())
        {
            m->source.error("voicebot_channel not defined.");
            return;
        }

        Channel::ptr channel = m->bot->find_channel(channelname);
        if (!channel)
        {
            m->source.error("Couldn't find channel " + channelname);
            return;
        }

        std::list<std::string> tovoice, tonotvoice;

        build_voice_lists(channel, &tovoice, &tonotvoice);

        while (!tovoice.empty())
        {
            std::list<std::string> thisvoicerun;

            int i;

            for (i = 0; i < m->bot->supported()->max_modes() && !tovoice.empty(); ++i)
            {
                thisvoicerun.push_back(*tovoice.begin());
                tovoice.pop_front();
            }
            std::string voicecommand = "MODE " + channelname + " " "+" + std::string(i, 'v') + " " +
                                       paludis::join(thisvoicerun.begin(), thisvoicerun.end(), " ");
            m->bot->send(voicecommand);
        }

        Logger::get_instance()->Log(m->bot, m->source.client, Logger::Command, "VOICE");
    }

    void do_match(const Message *m)
    {
        if (m->args.empty())
        {
            m->source.error("Need one argument");
            return;
        }

        std::string mask = m->args[0];
        if (mask.find_first_of("!@*") == std::string::npos)
        {
            Client::ptr c = m->bot->find_client(mask);
            if (!c)
                mask += "!*@*";
            else
                mask = c->nuh();
        }

        for (ValueArray::iterator it = dnv.begin(); it != dnv.end(); ++it)
            if (mask_match((*it)["mask"], mask))
            {
                Bot *bot = BotManager::get_instance()->find((*it)["bot"]);
                m->source.reply((*it)["mask"] + " (" + (*it)["reason"] + ") (added by " +
                    (*it)["setter"] + " on " + format_time(bot, (*it)["set"].Int()) +
                    ", expires " + format_time(bot, (*it)["expires"].Int()) + ")");
            }

        m->source.reply("*** End of DNV matches for " + mask);
    }

    void check_expiry()
    {
        time_t currenttime = time(NULL);

        for (ValueArray::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            if ((*it)["expires"].Int() != 0 && (*it)["expires"].Int() < currenttime)
            {
                Bot *bot = BotManager::get_instance()->find((*it)["bot"]);
                std::string adminchan;
                if (bot)
                    adminchan = bot->get_setting("voicebot_admin_channel");

                if (bot && !adminchan.empty())
                    bot->send("NOTICE " + adminchan + " :Removing expired entry " +
                            (*it)["mask"] + " added by " + (*it)["setter"] + " on " +
                            format_time(bot, (*it)["set"].Int()));

                old.push_back(*it);
                (*it)["removed"] = 1;
            }
        }

        do_removals(dnv);
    }

    void load_lists()
    {
        try
        {
            dnv = StorageManager::get_instance()->Load("donotvoice");
            old = StorageManager::get_instance()->Load("expireddonotvoice");
        }
        catch (StorageError &)
        {
            dnv = Value(Value::array);
            old = Value(Value::array);
        }
        catch (IOError &)
        {
            dnv = Value(Value::array);
            old = Value(Value::array);
        }
    }

    CommandHolder add, remove, list, info, check, voice, clear, change, match_client, shutdown;
    EventHolder check_event;
    HelpTopicHolder voicebothelp, voicehelp, checkhelp, matchhelp, addhelp, removehelp, edithelp;
    HelpIndexHolder index;

    voicebot()
        : dnv(GlobalSettingsManager::get_instance()->get("voicebot:donotvoice")),
          old(GlobalSettingsManager::get_instance()->get("voicebot:expireddonotvoice")),
          voicebothelp("voicebot", "voiceadmin", help_voicebot),
          voicehelp("voice", "voiceadmin", help_voice),
          checkhelp("check", "voiceadmin", help_check),
          matchhelp("match", "voiceadmin", help_match),
          addhelp("add", "voiceadmin", help_add),
          removehelp("remove", "voiceadmin", help_remove),
          edithelp("edit", "voiceadmin", help_edit),
          index("voicebot", "voiceadmin")
    {
        add = add_handler(filter_command_type("add", sourceinfo::IrcCommand).requires_privilege("voiceadmin"),
                            &voicebot::do_add);
        remove = add_handler(filter_command_type("remove", sourceinfo::IrcCommand).requires_privilege("voiceadmin"),
                            &voicebot::do_remove);
        list = add_handler(filter_command_type("list", sourceinfo::IrcCommand).requires_privilege("voiceadmin"),
                            &voicebot::do_list);
        check = add_handler(filter_command_type("check", sourceinfo::IrcCommand).requires_privilege("voiceadmin"),
                            &voicebot::do_check);
        voice = add_handler(filter_command_type("voice", sourceinfo::IrcCommand).requires_privilege("voiceadmin"),
                            &voicebot::do_voice);
        change = add_handler(filter_command_type("edit", sourceinfo::IrcCommand).requires_privilege("voiceadmin"),
                            &voicebot::do_change);
        match_client = add_handler(filter_command_type("match", sourceinfo::IrcCommand).requires_privilege("voiceadmin"),
                            &voicebot::do_match);

        check_event = add_recurring_event(60, &voicebot::check_expiry);

        StorageManager::get_instance()->auto_save(&dnv, "donotvoice");
        StorageManager::get_instance()->auto_save(&old, "expireddonotvoice");

        load_lists();
    }
};

MODULE_CLASS(voicebot)

