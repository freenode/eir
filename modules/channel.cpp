#include "eir.h"

#include <functional>

#include <paludis/util/tokeniser.hh>

#include <iostream>

using namespace eir;
using namespace std::tr1::placeholders;

struct ChannelHandler
{
    void handle_join(const Message *);
    void handle_part(const Message *);
    void handle_quit(const Message *);
    void handle_names_reply(const Message *);

    ChannelHandler();
    ~ChannelHandler();

    CommandRegistry::id join_id, part_id, quit_id, names_id;
};

ChannelHandler handles_channels;

ChannelHandler::ChannelHandler()
{
    CommandRegistry *r = CommandRegistry::get_instance();

    join_id = r->add_handler("JOIN", std::tr1::bind(std::tr1::mem_fn(&ChannelHandler::handle_join), this, _1));
    part_id = r->add_handler("PART", std::tr1::bind(std::tr1::mem_fn(&ChannelHandler::handle_part), this, _1));
    quit_id = r->add_handler("QUIT", std::tr1::bind(std::tr1::mem_fn(&ChannelHandler::handle_quit), this, _1));
    names_id = r->add_handler("353", std::tr1::bind(std::tr1::mem_fn(&ChannelHandler::handle_names_reply), this, _1));
}

ChannelHandler::~ChannelHandler()
{
    CommandRegistry *r = CommandRegistry::get_instance();
    r->remove_handler(join_id);
    r->remove_handler(part_id);
    r->remove_handler(quit_id);
    r->remove_handler(names_id);
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
    Client::ptr c = find_or_create_client(m);
    Channel::ptr ch = find_or_create_channel(m);

    c->join_chan(ch);
}

void ChannelHandler::handle_names_reply(const Message *m)
{
    std::string chname = m->args[1];
    std::vector<std::string> nicks;

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

