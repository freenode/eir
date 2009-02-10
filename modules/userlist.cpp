#include "eir.h"

#include "handler.h"

using namespace std::tr1::placeholders;
using namespace eir;

struct UserLister : public CommandHandlerBase<UserLister>
{
    void list(const Message *m)
    {
        if (! m->bot->supported()->is_channel_name(m->source.destination))
            return;

        Channel::ptr ch = m->bot->find_channel(m->source.destination);

        if (!ch)
            return;

        std::string reply;

        for (Channel::MemberIterator it = ch->begin_members(), ite = ch->end_members();
                it != ite; ++it)
        {
            if (m->args.empty() || (*it)->has_mode(m->args[0][0]))
                reply += (*it)->client->nick() + " ";
        }

        m->source.reply(reply);
    }

    CommandHolder _id;

    UserLister()
    {
        _id = add_handler("names", sourceinfo::IrcCommand, &UserLister::list);
    }
} lister;

