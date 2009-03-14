#include "eir.h"

#include "handler.h"

using namespace std::tr1::placeholders;
using namespace eir;

struct UserLister : CommandHandlerBase<UserLister>, Module
{
    void list(const Message *m)
    {
        std::string channelname;

        unsigned int modeindex = 0;

        if (m->bot->supported()->is_channel_name(m->source.destination))
            channelname = m->source.destination;

        if (!m->args.empty() && m->bot->supported()->is_channel_name(m->args[0]))
        {
            channelname = m->args[0];
            ++modeindex;
        }

        if (channelname.empty())
        {
            m->source.error("I need a channel name");
            return;
        }

        Channel::ptr ch = m->bot->find_channel(channelname);

        if (!ch)
        {
            m->source.error("I don't seem to be in that channel.");
            return;
        }

        std::string reply;

        for (Channel::MemberIterator it = ch->begin_members(), ite = ch->end_members();
                it != ite; ++it)
        {
            if (m->args.size() <= modeindex || (*it)->has_mode(m->args[modeindex][0]))
                reply += (*it)->client->nick() + " ";
        }

        if (reply.empty())
            m->source.reply("<nothing>");
        else
            m->source.reply(reply);
    }

    CommandHolder _id;

    UserLister()
    {
        _id = add_handler(filter_command_type("names", sourceinfo::IrcCommand), &UserLister::list);
    }
};

MODULE_CLASS(UserLister)

