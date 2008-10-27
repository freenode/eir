#include "bot.h"

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
    Message m;
    std::string::size_type p1, p2;

    std::string::iterator e = line.end();
    if (*--e == '\r')
        line.erase(e);

    p1 = line[0] == ':' ? 1 : 0;
    p2 = line.find(' ');
    m.source = line.substr(p1, p2 - p1);

    p1 = p2 + 1;
    p2 = line.find(' ', p1);
    m.command = line.substr(p1, p2 - p1);

    p1 = p2 + 1;
    p2 = line.find(' ', p1);
    m.destination = line.substr(p1, p2 - p1);

    while(p2 != std::string::npos)
    {
        p1 = p2 + 1;
        if (line[p1] == ':')
        {
            m.args.push_back(line.substr(p1, std::string::npos));
            break;
        }
        p2 = line.find(' ', p1);
        m.args.push_back(line.substr(p1, p2 - p1));
    }

    _dispatcher.dispatch(m);
}

void Bot::run()
{
    _server.connect(_host, _port);

    Message m("", "", "on_connect");
    _dispatcher.dispatch(m);

    if (_pass.length() > 0)
        _server.send("PASS " + _pass);

    _server.send("NICK " + _nick);
    _server.send("USER eir * * :eir version 0.0.1");

    _server.run();
}
