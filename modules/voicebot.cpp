#include "bot.h"
#include "handler.h"

using namespace eir;
using namespace std::tr1::placeholders;

#include <list>
#include "match.h"

#include <paludis/util/join.hh>
#include <time.h>

static const std::string default_time_fmt("%F %T");

static std::string format_time(Bot *b, time_t t)
{
    char datebuf[128];
    tm time;
    localtime_r(&t, &time);
    strftime(datebuf,
             sizeof(datebuf),
             b->get_setting_with_default("voice_time_fmt", default_time_fmt).c_str(),
             &time);
    return std::string(datebuf);
}

struct voicebot : public CommandHandlerBase<voicebot>
{
    struct voiceentry
    {
        std::string mask, setter, reason;
        time_t set, expires;
        voiceentry(std::string m, std::string s, std::string r, time_t st, time_t e)
            : mask(m), setter(s), reason(r), set(st), expires(e)
        { }
    };

    typedef std::list<voiceentry> voicelist;
    voicelist dnv;

    void do_add(const Message *m)
    {
        if (!m->source.client || !m->source.client->privs().has_privilege("voiceadmin"))
            return;

        if (m->args.empty())
        {
            m->source.error("Need at least one argument");
            return;
        }

        std::vector<std::string>::const_iterator it = m->args.begin();
        std::string mask = *it++;
        std::string reason = paludis::join(it, m->args.end(), " ");

        // If this looks like a plain nick instead of a mask, treat it as a nickname
        // mask.
        if (mask.find_first_of("!@*") == std::string::npos)
            mask += "!*@*";

        time_t expires = 0;

        for (voicelist::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            if (mask_match(it->mask, mask))
            {
                m->source.reply("Mask already matched by " + it->mask);
                return;
            }
        }

        dnv.push_back(voiceentry(mask, m->source.raw, reason, time(NULL), expires));
        m->source.reply("Added " + mask);
    }

    void do_remove(const Message *m)
    {
        if (!m->source.client || !m->source.client->privs().has_privilege("voiceadmin"))
            return;

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
                m->source.reply("Removing " + it->mask + " (" + it->reason + ") " +
                        "(added by " + it->setter + " on " + format_time(m->bot, it->set) + ")");

                dnv.erase(it++);
            }
            else
                ++it;
        }
    }

    void do_list(const Message *m)
    {
        if (!m->source.client || !m->source.client->privs().has_privilege("voiceadmin"))
            return;

        for (voicelist::iterator it = dnv.begin(); it != dnv.end(); ++it)
        {
            m->source.reply(it->mask + " (" + it->reason + ") (added by " + 
                    it->setter + " on " + format_time(m->bot, it->set) + ")");
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
        if (!m->source.client || !m->source.client->privs().has_privilege("voiceadmin"))
            return;

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
        if (!m->source.client || !m->source.client->privs().has_privilege("voiceadmin"))
            return;

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
    }

    CommandHolder add, remove, list, info, check, voice, clear;

    voicebot()
    {
        add = add_handler("add", sourceinfo::IrcCommand, &voicebot::do_add);
        remove = add_handler("remove", sourceinfo::IrcCommand, &voicebot::do_remove);
        list = add_handler("list", sourceinfo::IrcCommand, &voicebot::do_list);
        check = add_handler("check", sourceinfo::IrcCommand, &voicebot::do_check);
        voice = add_handler("voice", sourceinfo::IrcCommand, &voicebot::do_voice);
    }
} vb;

