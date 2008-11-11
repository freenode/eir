#include "bot.h"
#include "command.h"

using namespace eir;

using namespace std::tr1::placeholders;

Bot::Bot(std::string host, std::string port, std::string nick, std::string pass)
    : _server(std::tr1::bind(&Bot::handle_message, this, _1)),
      _host(host), _port(port), _nick(nick), _pass(pass)
{
}

Bot::~Bot()
{
}

void Bot::handle_message(std::string line)
{
    Message m(this);
    std::string::size_type p1, p2;

    std::string::iterator e = line.end();
    if (*--e == '\r')
        line.erase(e);

    /*
    p1 = line[0] == ':' ? 1 : 0;
    p2 = line.find(' ');
    m.source = line.substr(p1, p2 - p1);
    */
    if (line[0] == ':')
    {
        p1 = 1;
        p2 = line.find(' ');
        m.source.raw = line.substr(p1, p2 - p1);
        p1 = p2 + 1;
    }
    else
    {
        m.source.raw = "";
        p1 = 0;
    }

    std::string::size_type bang = m.source.raw.find('!');
    if (bang != std::string::npos)
    {
        std::string nick = m.source.raw.substr(bang);
        ClientIterator c = find_client(nick);
        if (c != _clients.end())
            m.source.client = c->second;
        else
            m.source.client = 0;

        m.source.name = nick;
    }
    else
    {
        m.source.client = 0;
        m.source.name = m.source.raw;
    }

    p2 = line.find(' ', p1);
    m.command = line.substr(p1, p2 - p1);

    p1 = p2 + 1;
    p2 = line.find(' ', p1);
    std::string destination = line.substr(p1, p2 - p1);

    if (destination[0] == '#' || destination[0] == '&')
    {
        m.source.in_channel = true;
        m.source.channel = destination;
    }
    else
        m.source.in_channel = false;

    while(p2 != std::string::npos)
    {
        p1 = p2 + 1;
        if (line[p1] == ':')
        {
            m.args.push_back(line.substr(p1+1, std::string::npos));
            break;
        }
        p2 = line.find(' ', p1);
        m.args.push_back(line.substr(p1, p2 - p1));
    }

    CommandRegistry::get_instance()->dispatch(&m);
}

void Bot::run()
{
    _server.connect(_host, _port);

    Message m(this, "on_connect");
    CommandRegistry::get_instance()->dispatch(&m);

    if (_pass.length() > 0)
        _server.send("PASS " + _pass);

    _server.send("NICK " + _nick);
    _server.send("USER eir * * :eir version 0.0.1");

    _server.run();
}

void Bot::send(std::string line)
{
    _server.send(line);
}

void Bot::_handle_join(const Message *m)
{
    Client *c = m->source.client;

    if (!c)
    {
        ClientIterator cli = _clients.find(m->source.nick);
        if (cli != _clients.end())
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
            // What? Something that's not a user just joined a channel.
            throw std::exception("Something that's not a user just joined a channel. I'm confused.");
        }
        nick = m->source.raw.substr(bang);
        at = m->source.raw.find('@', bang + 1);
        user = m->source.raw.substr(bang + 1, at - bang - 1);
        host = m->source.raw.substr(at + 1, std::string::npos);

        c = new Client(nick, user, host);
        _clients.insert(nick, c);
    }

    Channel *ch = 0;
    ChannelIterator chi = _channels.find(m->source.channel);
    if (chi != _channels.end())
    {
        ch = chi->second;
    }
    else
    {
        ch = new Channel(m->source.channel);
        _channels.insert(m->source.channel, ch);
    }

    c->join_chan(ch);
}



