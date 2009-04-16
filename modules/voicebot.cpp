#include "eir.h"

using namespace eir;
using namespace std::tr1::placeholders;

#include <list>
#include "match.h"

#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>

#include "times.h"

#include <fstream>

static const std::string default_time_fmt("%F %T");
static const std::string default_expiry("1d");

static std::string format_time(Bot *b, time_t t)
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

static time_t get_default_expiry(Bot *b)
{
    std::string time = b->get_setting_with_default("default_voice_expiry", default_expiry);
    return parse_time(time);
}

struct voicebot : CommandHandlerBase<voicebot>, Module
{
    struct voiceentry
    {
        std::string bot;
        std::string mask, setter, reason;
        time_t set, expires;
        voiceentry(std::string b, std::string m, std::string s, std::string r, time_t st, time_t e)
            : bot(b), mask(m), setter(s), reason(r), set(st), expires(e)
        { }
        voiceentry()
            : set(0), expires(0)
        { }
    };

    typedef std::list<voiceentry> voicelist;
    voicelist dnv;
    voicelist old;

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

        for (voicelist::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            if (mask_match(it->mask, mask))
            {
                m->source.reply("Mask already matched by " + it->mask);
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
            expires = time(NULL) + parse_time(*it++);

        std::string reason = paludis::join(it, m->args.end(), " ");

        // If this looks like a plain nick instead of a mask, treat it as a nickname
        // mask.
        if (mask.find_first_of("!@*") == std::string::npos)
            mask += "!*@*";

        bool found = false;

        for (voicelist::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            if (mask_match(mask, it->mask))
            {
                if (expires)
                    it->expires = expires;
                if (!reason.empty())
                    it->reason = reason;
                found = true;
                m->source.reply("Updated " + it->mask);
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

        for (voicelist::iterator it = dnv.begin(); it != dnv.end(); )
        {
            if (mask_match(mask, it->mask))
            {
                Bot *bot = BotManager::get_instance()->find(it->bot);
                m->source.reply("Removing " + it->mask + " (" + it->reason + ") " +
                        "(added by " + it->setter + " on " + format_time(bot, it->set) + ")");

                old.push_back(*it);
                dnv.erase(it++);
            }
            else
                ++it;
        }

        Logger::get_instance()->Log(m->bot, m->source.client, Logger::Command, "REMOVE " + mask);
    }

    void do_list(const Message *m)
    {
        for (voicelist::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            Bot *bot = BotManager::get_instance()->find(it->bot);
            m->source.reply(it->mask + " (" + it->reason + ") (added by " + 
                    it->setter + " on " + format_time(bot, it->set) +
                    ", expires " + format_time(bot, it->expires) + ")");
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
            for (voicelist::iterator i2 = dnv.begin(); i2 != dnv.end(); ++i2)
            {
                if (match(i2->mask, (*it)->client->nuh()))
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

        for (voicelist::iterator it = dnv.begin(); it != dnv.end(); ++it)
            if (mask_match(it->mask, mask))
            {
                Bot *bot = BotManager::get_instance()->find(it->bot);
                m->source.reply(it->mask + " (" + it->reason + ") (added by " +
                    it->setter + " on " + format_time(bot, it->set) +
                    ", expires " + format_time(bot, it->expires) + ")");
            }

        m->source.reply("*** End of DNV matches for " + mask);
    }

    void check_expiry()
    {
        time_t currenttime = time(NULL);

        for (voicelist::iterator it = dnv.begin(); it != dnv.end(); )
        {
            if (it->expires != 0 && it->expires < currenttime)
            {
                Bot *bot = BotManager::get_instance()->find(it->bot);
                std::string adminchan;
                if (bot)
                    adminchan = bot->get_setting("voicebot_admin_channel");

                if (bot && !adminchan.empty())
                    bot->send("NOTICE " + adminchan + " :Removing expired entry " +
                            it->mask + " added by " + it->setter + " on " + format_time(bot, it->set));

                old.push_back(*it);
                dnv.erase(it++);
            }
            else
                ++it;
        }
    }

    void load_voicelist(std::string filename, voicelist *list)
    {
        std::ifstream file(filename.c_str());

        std::string line;

        list->clear();

        while (std::getline(file, line))
        {
            std::vector<std::string> tokens;
            paludis::tokenise_whitespace(line, std::back_inserter(tokens));

            if (tokens.size() < 5)
                continue;

            std::vector<std::string>::iterator it = tokens.begin();
            voiceentry e;
            e.bot = *it++;
            e.mask = *it++;
            e.setter = *it++;
            e.set = atoi((*it++).c_str());
            e.expires = atoi((*it++).c_str());
            if (tokens.size() > 5)
                e.reason = paludis::join(it, tokens.end(), " ");

            list->push_back(e);
        }
    }

    void save_voicelist(std::string filename, voicelist *list)
    {
        std::string tmpfilename = filename + ".tmp";
        std::ofstream file(tmpfilename.c_str());

        for (voicelist::iterator it = list->begin(); it != list->end(); ++it)
        {
            file << it->bot << " " << it->mask << " " << it->setter << " "
                 << it->set << " " << it->expires << " " << it->reason << std::endl;
        }

        file.close();
        ::rename(tmpfilename.c_str(), filename.c_str());
    }

    void load_lists()
    {
        load_voicelist("voice.db", &dnv);
        load_voicelist("expired.voice.db", &old);
    }

    void save_lists()
    {
        save_voicelist("voice.db", &dnv);
        save_voicelist("expired.voice.db", &old);
    }


    CommandHolder add, remove, list, info, check, voice, clear, change, match_client;
    EventHolder check_event, save_event;

    voicebot()
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
        save_event = add_recurring_event(300, &voicebot::save_lists);

        load_lists();
    }

    ~voicebot()
    {
        try
        {
            save_lists();
        }
        catch (...)
        {
        }
    }
};

MODULE_CLASS(voicebot)

