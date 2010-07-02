#include "eir.h"

using namespace eir;
using namespace std::placeholders;

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

    time_t get_revoice_expiry(Bot *b)
    {
        std::string time = b->get_setting_with_default("revoice_expiry", default_expiry);
        return parse_time(time);
    }

    const char *help_voicebot =
        "The voicebot module exists to manage a moderated channel. It maintains a list of hostmask entries that "
        "will not be voiced (the \002DNV\002 list), and voices, on request, all unvoiced users in the channel that "
        "do not match any of those entries.\n"
        "If voicebot_enable_revoicing is set users who leave the channel while voiced will be revoiced automatically "
        "should they return before the time specified in revoice_expiry has elapsed.\n "
        "Available voicebot commands are: voice check match add remove edit.\n "
        "Relevant settings are \037voicebot_channel\037, \037voicebot_admin_channel\037, \037default_voice_expiry\037, "
        "\037voicebot_enable_revoicing\037, \037revoice_expiry\037 and \037voice_time_format\037.";
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

    Value lostvoiceentry(std::string bot, std::string mask, time_t expires)
    {
        Value v(Value::kvarray);
        v["bot"] = bot;
        v["mask"] = mask;
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
    Value &dnv, &old, &lostvoices;

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
        for (ValueArray::iterator it = lostvoices.begin(); it != lostvoices.end(); ++it)
        {
            if ((*it)["expires"].Int() != 0 && (*it)["expires"].Int() < currenttime)
            {
                (*it)["removed"] = 1;
            }
        }

        do_removals(dnv);
        do_removals(lostvoices);
    }

    void load_list(Value & v, std::string name)
    {
        try
        {
            v = StorageManager::get_instance()->Load(name);
            if (v.Type() != Value::array)
                throw "wrong type";
        }
        catch (StorageError &)
        {
            v = Value(Value::array);
        }
        catch (IOError &)
        {
            v = Value(Value::array);
        }
        catch (const char *)
        {
            Logger::get_instance()->Log(NULL, NULL, Logger::Warning,
                    "Loaded voice list " + name + "has wrong type; ignoring");
            v = Value(Value::array);
        }
    }

    void load_lists()
    {
        load_list(dnv, "donotvoice");
        load_list(old, "expireddonotvoice");
        load_list(lostvoices, "lostvoices");
    }

    std::string build_revoice_mask (Client::ptr c)
    {
    /* This is freenode specific, so may need to be changed if running on a network
       with different vhost policies

       in English:

       Uncloaked users are matched on their full nick!user@host mask
       Gateway cloaked users are matched on nick!user@gateway/type/name, minus the x-NNNNNNNNNNNNNNNN session ID
       Authenticated gateway cloaked users (currently only tor-sasl), and regular cloaked users are matched on *!*@cloak

    */

        if (c->host().find("/") == std::string::npos)
        {
            // normal user, return full nuh
            return c->nuh();
        } else if (c->host().find("gateway/tor-sasl/") == 0) {
            // tor-sasl user, return *!*@cloak
            return "*!*@" + c->host();
        } else if (c->host().find("gateway/") == 0 ||
                   c->host().find("conference/") == 0 ||
                   c->host().find("nat/") == 0 )
        {
            // gateway user
            std::string suffix=c->nuh().substr(c->nuh().find_last_of("/"));
            if (suffix=="/session" || suffix.substr(0,3) == "/x-" || suffix.substr(0,4)== "/ip.")
            {
                // strip session ID
                return c->nuh().substr(0,c->nuh().find_last_of("/")) + "/*";
            } else {
                // got an unrecognised suffix - don't return a revoice mask
                return "";
            }
        } else {
            // cloaked user, return *!*@cloak
            return "*!*@" + c->host();
        }
    }



    void irc_join(const Message *m)
    {
        std::string revoicing = m->bot->get_setting("voicebot_enable_revoicing");
        std::string channelname = m->bot->get_setting("voicebot_channel");

        if (m->source.name == m->bot->nick())
        {
            return;
        }

        if (m->source.destination != channelname)
        {
            return;
        }

        for (ValueArray::iterator it = lostvoices.begin(); it != lostvoices.end(); ++it)
        {
            if (mask_match((*it)["mask"], m->source.raw))
            {
                Client::ptr c = m->bot->find_client(m->source.name);
                if (c)
                {
                    std::weak_ptr<Client> w(c);
                    Logger::get_instance()->Log(m->bot, NULL, Logger::Debug, "*** Matched lost voice for " + m->source.raw + "(" + (*it)["mask"] + ")");
                    Logger::get_instance()->Log(m->bot, NULL, Logger::Debug, "*** Queueing revoice for " + m->source.name);
                    add_event(time(NULL)+5, std::bind(revoice, m->bot, w, it, lostvoices, channelname));
                }
            }
        }
    }

    void irc_nick(const Message *m)
    {
        std::string revoicing = m->bot->get_setting("voicebot_enable_revoicing");
        std::string channelname = m->bot->get_setting("voicebot_channel");

        if (m->source.name == m->bot->nick())
        {
            return;
        }

        for (ValueArray::iterator it = lostvoices.begin(); it != lostvoices.end(); ++it)
        {
            if (mask_match((*it)["mask"], m->source.client->nuh()))
            {
                std::weak_ptr<Client> w(m->source.client);
                Logger::get_instance()->Log(m->bot, NULL, Logger::Debug, "*** Matched lost voice for " + m->source.raw + "(" + (*it)["mask"] + ")");
                Logger::get_instance()->Log(m->bot, NULL, Logger::Debug, "*** Queueing revoice for " + m->source.destination );
                add_event(time(NULL)+5, std::bind(revoice, m->bot, w, it, lostvoices, channelname));
            }
        }
    }

    void irc_depart (const Message *m)
    {
        std::string revoicing = m->bot->get_setting("voicebot_enable_revoicing");
        std::string channelname = m->bot->get_setting("voicebot_channel");

        if (m->source.name == m->bot->nick())
        {
            return;
        }

        Channel::ptr channel = m->bot->find_channel(channelname);
        if (!channel)
        {
            return;
        }

        Membership::ptr mem = m->source.client->find_membership(channelname);
        if (mem && mem->has_mode('v')) {
            if (m->command == "PART" && m->source.destination == channelname )
            {
                if (m->args.size() >= 1 && m->args[0].substr(0,9) == "requested")
                {
                    // user was ejected from the channel with REMOVE
                    Logger::get_instance()->Log(m->bot, NULL, Logger::Debug, "*** " + m->source.client->nick()  + "was removed from channel - will not revoice");
                    return;
                }
            } else if (m->command == "QUIT") {
                if (!m->source.destination.empty())
                {
                    std::string q = m->source.destination.substr(0,m->source.destination.find(" "));
                    if (q == "Killed" ||  q == "K-Lined" ||  q == "Changing" ||  q == "*.net")
                    {
                        // Abnormal quit - ignore
                        Logger::get_instance()->Log(m->bot, NULL, Logger::Debug, "*** " + m->source.client->nick()  + "left network abnormally - will not revoice");
                        return;
                    }
                }
            } else {
                // Wrong channel, or something odd happened
                return;
            }
            // User left the channel or network normally while voiced - put them on the lost voices list
            std::string mask = m->source.raw;
            mask=build_revoice_mask(m->source.client);
            if (!mask.empty())
            {
                // check we don't already have this mask
                for (ValueArray::iterator it = lostvoices.begin(); it != lostvoices.end(); ++it)
                {
                    if ((*it)["mask"] == mask)
                    {
                        Logger::get_instance()->Log(m->bot, NULL, Logger::Debug, "*** " + mask + " is already on lostvoices list, skipping");
                        return;
                    }
                }
                lostvoices.push_back(lostvoiceentry(m->bot->name(), mask, get_revoice_expiry(m->bot)+time(NULL)));
                Logger::get_instance()->Log(m->bot, NULL, Logger::Debug, "*** " + m->source.client->nick() + "(" + mask + ")" + " left " + channelname + " with voice");
            }
        }
    }

    static void revoice(Bot *bot, std::weak_ptr<Client> w, ValueArray::iterator it, Value& lv, std::string channel)
    {
        Client::ptr c = w.lock();
        if (!c)
        {
            return;
        }

        Membership::ptr mem=c->find_membership(channel);
        if (!mem)
        {
            return;
        }

        if (mem->has_mode('v'))
        {
            Logger::get_instance()->Log(bot, NULL, Logger::Debug, "**** " + c->nick() + " is alreadly voiced on " + channel +", skipping");
        } else {
            Logger::get_instance()->Log(bot, NULL, Logger::Debug, "*** Revoicing " + c->nick() + " on "+ channel);
            Logger::get_instance()->Log(bot, NULL, Logger::Admin, "*** Revoicing " + c->nick() + " on "+ channel);
            bot->send("MODE " + channel + " +v " + c->nick());
            if (it != lv.end()) {
                (*it)["removed"]=1;
                do_removals(lv);
            }
        }
    }

    CommandHolder add, remove, list, info, check, voice, clear, change, match_client, shutdown, join, part, quit, nick;
    EventHolder check_event;
    HelpTopicHolder voicebothelp, voicehelp, checkhelp, matchhelp, addhelp, removehelp, edithelp;
    HelpIndexHolder index;

    voicebot()
        : dnv(GlobalSettingsManager::get_instance()->get("voicebot:donotvoice")),
          old(GlobalSettingsManager::get_instance()->get("voicebot:expireddonotvoice")),
          lostvoices(GlobalSettingsManager::get_instance()->get("voicebot:lostvoices")),
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
        quit = add_handler(filter_command_type("QUIT", sourceinfo::RawIrc),&voicebot::irc_depart, true, Message::first);
        part = add_handler(filter_command_type("PART", sourceinfo::RawIrc),&voicebot::irc_depart, true, Message::first);
        join = add_handler(filter_command_type("JOIN", sourceinfo::RawIrc),&voicebot::irc_join,true);
        nick = add_handler(filter_command_type("NICK", sourceinfo::RawIrc),&voicebot::irc_nick,true);

        check_event = add_recurring_event(60, &voicebot::check_expiry);

        StorageManager::get_instance()->auto_save(&dnv, "donotvoice");
        StorageManager::get_instance()->auto_save(&old, "expireddonotvoice");
        StorageManager::get_instance()->auto_save(&lostvoices, "lostvoices");

        load_lists();
    }
};

MODULE_CLASS(voicebot)

