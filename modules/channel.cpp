#include "eir.h"

#include <functional>

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
//    names_id = r->add_handler("353", std::tr1::bind(std::tr1::mem_fn(&ChannelHandler::handle_names_reply), this, _1));
}

ChannelHandler::~ChannelHandler()
{
    CommandRegistry *r = CommandRegistry::get_instance();
    r->remove_handler(join_id);
    r->remove_handler(part_id);
    r->remove_handler(quit_id);
    r->remove_handler(names_id);
}


void ChannelHandler::handle_join(const Message *m)
{
    Client::ptr c = m->source.client;
    Bot *b = m->bot;

    if (!c)
    {
        Bot::ClientIterator cli = b->find_client(m->source.name);
        if (cli != b->end_clients())
            c = cli->second;
    }
    if(!c)
    {
        // We don't know anything about this client. Make a new client struct and join it.
        std::string nick, user, host;
        std::string::size_type bang, at;
        bang = m->source.raw.find('!');
        if(bang == std::string::npos)
        {
            // We don't know this client's user@host yet. Leave it blank.
            nick = m->source.raw;
            user = "";
            host = "";
        }
        else
        {
            nick = m->source.raw.substr(0, bang);
            at = m->source.raw.find('@', bang + 1);
            user = m->source.raw.substr(bang + 1, at - bang - 1);
            host = m->source.raw.substr(at + 1, std::string::npos);
        }

        c.reset(new Client(nick, user, host));
        b->add_client(c);
    }

    Channel::ptr ch;
    Bot::ChannelIterator chi = b->find_channel(m->source.channel);
    if (chi != b->end_channels())
    {
        ch = chi->second;
    }
    else
    {
        ch.reset(new Channel(m->source.channel));
        b->add_channel(ch);
    }

    c->join_chan(ch);

    std::cerr << "JOIN: " << c->nick() << " to " << ch->name() << std::endl;
}

void ChannelHandler::handle_part(const Message *m)
{
    Client::ptr c = m->source.client;
    Bot *b = m->bot;

    // If we don't know anything about the client, then they can't be in our channel lists.
    if (!c)
        return;

    Bot::ChannelIterator chi = b->find_channel(m->source.channel);
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

