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

    m.raw = line;

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
        std::string nick = m.source.raw.substr(0, bang);
        ClientIterator c = find_client(nick);
        if (c != _clients.end())
            m.source.client = c->second;

        m.source.name = nick;
    }
    else
    {
        m.source.name = m.source.raw;
    }

    p2 = line.find(' ', p1);
    m.command = line.substr(p1, p2 - p1);

    p1 = p2 + 1;
    p2 = line.find(' ', p1);
    m.source.destination = line.substr(p1, p2 - p1);

    if(m.source.destination[0] == ':')
    {
        m.source.destination = line.substr(p1 + 1);
        p2 = std::string::npos;
    }

    if (m.source.destination.find_first_of("#&") != std::string::npos)
    {
        m.source.in_channel = true;
        m.source.channel = m.source.destination;
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

void Bot::_init_me(const Message *m)
{
    _nick = m->source.destination;
    Client::ptr c(new Client(_nick, "", ""));
    _me = c;
    _clients.insert(make_pair(_nick, _me));
    send("USERHOST " + _nick);
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

