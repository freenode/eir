#include "eir.h"
#include "handler.h"

#include <functional>

#include <paludis/util/tokeniser.hh>

#include <paludis/util/wrapped_forward_iterator-impl.hh>

#include <iostream>

using namespace eir;
using namespace std::tr1::placeholders;

struct ChannelHandler : public CommandHandlerBase<ChannelHandler>
{
    void handle_join(const Message *);
    void handle_part(const Message *);
    void handle_quit(const Message *);
    void handle_names_reply(const Message *);
    void handle_nick(const Message *);
    void handle_who_reply(const Message *);

    ChannelHandler();
    ~ChannelHandler();

    CommandRegistry::id join_id, part_id, quit_id, names_id, nick_id, who_id;
};

ChannelHandler handles_channels;

ChannelHandler::ChannelHandler()
{
    join_id = add_handler("JOIN", sourceinfo::RawIrc, &ChannelHandler::handle_join);
    part_id = add_handler("PART", sourceinfo::RawIrc, &ChannelHandler::handle_part);
    quit_id = add_handler("QUIT", sourceinfo::RawIrc, &ChannelHandler::handle_quit);
    //names_id = add_handler("353", sourceinfo::RawIrc, &ChannelHandler::handle_names_reply);
    nick_id = add_handler("NICK", sourceinfo::RawIrc, &ChannelHandler::handle_nick);
    who_id = add_handler("352", sourceinfo::RawIrc, &ChannelHandler::handle_who_reply);
}

ChannelHandler::~ChannelHandler()
{
    remove_handler(join_id);
    remove_handler(part_id);
    remove_handler(quit_id);
    remove_handler(names_id);
    remove_handler(nick_id);
}

namespace
{
    Client::ptr find_or_create_client(Bot *b, std::string nick, std::string user, std::string host)
    {
        Client::ptr c = b->find_client(nick);

        if(!c)
        {
            c.reset(new Client(b, nick, user, host));
            b->add_client(c);
        }

        return c;
    }

    Client::ptr find_or_create_client(Bot *b, std::string name, std::string nuh)
    {
        Client::ptr c = b->find_client(name);

        if(!c)
        {
            // We don't know anything about this client. Make a new client struct and join it.
            std::string nick, user, host;
            std::string::size_type bang, at;
            bang = nuh.find('!');
            if(bang == std::string::npos)
            {
                // We don't know this client's user@host yet. Leave it blank.
                nick = nuh;
                user = "";
                host = "";
            }
            else
            {
                nick = nuh.substr(0, bang);
                at = nuh.find('@', bang + 1);
                user = nuh.substr(bang + 1, at - bang - 1);
                host = nuh.substr(at + 1, std::string::npos);
            }

            c.reset(new Client(b, nick, user, host));
            b->add_client(c);
        }

        return c;
    }

    Client::ptr find_or_create_client(const Message *m)
    {
        Client::ptr c = m->source.client;
        if (c)
            return c;

        return find_or_create_client(m->bot, m->source.name, m->source.raw);
    }

    Channel::ptr find_or_create_channel(Bot *b, std::string name)
    {
        Channel::ptr ch = b->find_channel(name);

        if (!ch)
        {
            ch.reset(new Channel(name));
            b->add_channel(ch);
        }
        return ch;
    }

    Channel::ptr find_or_create_channel(const Message *m)
    {
        return find_or_create_channel(m->bot, m->source.destination);
    }
}

void ChannelHandler::handle_join(const Message *m)
{
    Context ctx("Processing join for " + m->source.name + " to " + m->source.destination);

    Client::ptr c = find_or_create_client(m);
    Channel::ptr ch = find_or_create_channel(m);

    c->join_chan(ch);

    if (m->source.name == m->bot->nick())
        m->bot->send("WHO " + m->source.destination);
}

void ChannelHandler::handle_names_reply(const Message *m)
{
    if (m->args.size() < 2)
        return;

    std::string chname = m->args[1];
    std::vector<std::string> nicks;

    Context ctx("Processing NAMES reply for " + chname);

    paludis::tokenise_whitespace(m->args[2], std::back_inserter(nicks));

    Channel::ptr ch = find_or_create_channel(m->bot, chname);

    for (std::vector<std::string>::iterator it = nicks.begin(), ite = nicks.end();
            it != ite; ++it)
    {
        Client::ptr c = find_or_create_client(m->bot, *it, *it);
        c->join_chan(ch);
    }
}

void ChannelHandler::handle_who_reply(const Message *m)
{
    if (m->args.size() != 7) return;

    std::string chname = m->args[0],
                user = m->args[1],
                hostname = m->args[2],
                /* server = m->args[3], */
                nick = m->args[4],
                flags = m->args[5];

    Context ctx("Processing WHO reply for " + chname + " (" + nick + ")");
    Client::ptr c = find_or_create_client(m->bot, nick, user, hostname);
    Channel::ptr ch = find_or_create_channel(m->bot, chname);
    Membership::ptr member = c->join_chan(ch);

    for (std::string::iterator ch = flags.begin(); ch != flags.end(); ++ch)
    {
        char c = m->bot->supported()->get_prefix_mode(*ch);
        if (c && !member->has_mode(c))
            member->modes += c;
    }
}

void ChannelHandler::handle_part(const Message *m)
{
    Context ctx("Processing part for " + m->source.name + " from " + m->source.destination);

    Client::ptr c = m->source.client;
    Bot *b = m->bot;

    // If we don't know anything about the client, then they can't be in our channel lists.
    if (!c)
        return;

    Channel::ptr ch = b->find_channel(m->source.destination);
    if(!ch)
        return;

    c->leave_chan(ch);

    if(c->begin_channels() == c->end_channels())
    {
        // If they don't share any channels, we can't know anything about them.
        b->remove_client(c);
    }

    // We don't need to check whether ch is now empty, as we wouldn't be
    // getting this message if we're not in it.
}

void ChannelHandler::handle_quit(const Message *m)
{
    Context ctx("Handling quit from " + m->source.name);

    Client::ptr c = m->source.client;
    Bot *b = m->bot;

    if (!c)
        return;

    for (Client::ChannelIterator chi = c->begin_channels(), che = c->end_channels();
            chi != che; ++chi)
        c->leave_chan(*chi);

    b->remove_client(c);

    std::cerr << "QUIT: " << c->nick() << std::endl;
}

void ChannelHandler::handle_nick(const Message *m)
{
    Context ctx("Handling nick change from " + m->source.name);

    if(!m->source.client)
        return;
    std::string newnick(m->source.destination);
    m->source.client->change_nick(newnick);
}

