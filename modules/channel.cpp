#include "eir.h"
#include "handler.h"

#include <functional>

#include <paludis/util/tokeniser.hh>

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

    ChannelHandler();
    ~ChannelHandler();

    CommandRegistry::id join_id, part_id, quit_id, names_id, nick_id;
};

ChannelHandler handles_channels;

ChannelHandler::ChannelHandler()
{
    join_id = add_handler("JOIN", sourceinfo::RawIrc, &ChannelHandler::handle_join);
    part_id = add_handler("PART", sourceinfo::RawIrc, &ChannelHandler::handle_part);
    quit_id = add_handler("QUIT", sourceinfo::RawIrc, &ChannelHandler::handle_quit);
    names_id = add_handler("353", sourceinfo::RawIrc, &ChannelHandler::handle_names_reply);
    nick_id = add_handler("NICK", sourceinfo::RawIrc, &ChannelHandler::handle_nick);
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
    Client::ptr find_or_create_client(Bot *b, std::string name, std::string nuh)
    {
        Client::ptr c;

        Bot::ClientIterator cli = b->find_client(name);
        if (cli != b->end_clients())
            c = cli->second;

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

            c.reset(new Client(nick, user, host));
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
        Channel::ptr ch;
        Bot::ChannelIterator chi = b->find_channel(name);
        if (chi != b->end_channels())
        {
            ch = chi->second;
        }
        else
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
}

void ChannelHandler::handle_names_reply(const Message *m)
{
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

void ChannelHandler::handle_part(const Message *m)
{
    Context ctx("Processing part for " + m->source.name + " from " + m->source.destination);

    Client::ptr c = m->source.client;
    Bot *b = m->bot;

    // If we don't know anything about the client, then they can't be in our channel lists.
    if (!c)
        return;

    Bot::ChannelIterator chi = b->find_channel(m->source.destination);
    if(chi == b->end_channels())
        return;

    Channel::ptr ch = chi->second;

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
    m->source.client->change_nick(m->args[0]);
}

