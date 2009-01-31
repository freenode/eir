#include "bot.h"
#include "command.h"
#include "exceptions.h"

#include "server.h"

#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace eir;

using namespace std::tr1::placeholders;
using paludis::Implementation;

template class paludis::WrappedForwardIterator<Bot::ClientIteratorTag, const std::pair<const std::string, Client::ptr> >;
template class paludis::WrappedForwardIterator<Bot::ChannelIteratorTag, const std::pair<const std::string, Channel::ptr> >;
template class paludis::WrappedForwardIterator<Bot::SettingsIteratorTag, const std::pair<const std::string, std::string> >;

namespace paludis
{
    template <>
    struct Implementation<Bot>
    {
        typedef std::map<std::string, Client::ptr> ClientMap;
        typedef std::map<std::string, Channel::ptr> ChannelMap;
        typedef std::map<std::string, std::string> SettingsMap;

        Bot *bot;

        Server _server;
        std::string _host, _port, _nick, _pass;

        Client::ptr _me;

        ClientMap _clients;
        ChannelMap _channels;
        SettingsMap _settings;

        ISupport _supported;

        void handle_message(std::string);

        CommandRegistry::id init_handler;
        void _init_me(const Message *);

        CommandRegistry::id set_handler;
        void handle_set(const Message *);

        Implementation(Bot *b, std::string host, std::string port, std::string nick, std::string pass)
            : bot(b), _server(std::tr1::bind(&Implementation<Bot>::handle_message, this, _1)),
              _host(host), _port(port), _nick(nick), _pass(pass)
        {
            set_handler = CommandRegistry::get_instance()->add_handler("set",
                    std::tr1::bind(&Implementation<Bot>::handle_set, this, _1));
        }
    };
}


Bot::Bot(std::string host, std::string port, std::string nick, std::string pass)
    : paludis::PrivateImplementationPattern<Bot>(new paludis::Implementation<Bot>(this, host, port, nick, pass))
{
}

Bot::~Bot()
{
}

static void notice_to(Bot *b, std::string dest, std::string text)
{
    b->send("NOTICE " + dest + " :" + text);
}

void paludis::Implementation<Bot>::handle_message(std::string line)
{
    Context c("Parsing message " + line);

    Message m(bot);
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
        Bot::ClientIterator c = bot->find_client(nick);
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
        m.source.reply_func = std::tr1::bind(notice_to, bot, m.source.destination, _1);
    else
        m.source.reply_func = std::tr1::bind(notice_to, bot, m.source.name, _1);

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

void paludis::Implementation<Bot>::_init_me(const Message *m)
{
    _nick = m->source.destination;
    Client::ptr c(new Client(_nick, "", ""));
    _me = c;
    _clients.insert(make_pair(_nick, _me));
    bot->send("USERHOST " + _nick);
}

void paludis::Implementation<Bot>::handle_set(const Message *m)
{
    if (m->source.special != sourceinfo::ConfigFile)
        return;

    if (m->args.size() < 2)
    {
        m->source.reply("Not enough parameters to SET -- need two");
        return;
    }

    _settings[m->args[0]] = m->args[1];
}

const std::string& Bot::nick() const
{
    return _imp->_nick;
}

const Client::ptr Bot::me() const
{
    return _imp->_me;
}

void Bot::run()
{
    _imp->_server.connect(_imp->_host, _imp->_port);

    Message m(this, "on_connect");
    CommandRegistry::get_instance()->dispatch(&m);

    if (_imp->_pass.length() > 0)
        _imp->_server.send("PASS " + _imp->_pass);

    _imp->_server.send("NICK " + _imp->_nick);
    _imp->_server.send("USER eir * * :eir version 0.0.1");

    _imp->_server.run();
}

void Bot::send(std::string line)
{
    _imp->_server.send(line);
}

// Client stuff

Bot::ClientIterator Bot::begin_clients()
{
    return _imp->_clients.begin();
}

Bot::ClientIterator Bot::end_clients()
{
    return _imp->_clients.end();
}

Bot::ClientIterator Bot::find_client(std::string nick)
{
    return _imp->_clients.find(nick);
}

std::pair<Bot::ClientIterator, bool> Bot::add_client(Client::ptr c)
{
    Context ctx("Adding client " + c->nick());
    return _imp->_clients.insert(make_pair(c->nick(), c));
}

unsigned long Bot::remove_client(Client::ptr c)
{
    Context ctx("Removing client " + c->nick());
    return _imp->_clients.erase(c->nick());
}

// Channel stuff

Bot::ChannelIterator Bot::begin_channels()
{
    return _imp->_channels.begin();
}

Bot::ChannelIterator Bot::end_channels()
{
    return _imp->_channels.end();
}

Bot::ChannelIterator Bot::find_channel(std::string name)
{
    return _imp->_channels.find(name);
}

std::pair<Bot::ChannelIterator, bool> Bot::add_channel(Channel::ptr c)
{
    Context ctx("Adding channel " + c->name());
    return _imp->_channels.insert(make_pair(c->name(), c));
}

unsigned long Bot::remove_channel(Channel::ptr c)
{
    Context ctx("Removing channel " + c->name());
    return _imp->_channels.erase(c->name());
}

void Bot::remove_channel(Bot::ChannelIterator c)
{
    Context ctx("Removing channel " + c->second->name());
    _imp->_channels.erase(c.underlying_iterator<Implementation<Bot>::ChannelMap::iterator>());
}

// Settings stuff

Bot::SettingsIterator Bot::begin_settings()
{
    return _imp->_settings.begin();
}

Bot::SettingsIterator Bot::end_settings()
{
    return _imp->_settings.end();
}

Bot::SettingsIterator Bot::find_setting(std::string name)
{
    return _imp->_settings.find(name);
}

std::pair<Bot::SettingsIterator, bool> Bot::add_setting(std::string n, std::string s)
{
    Context ctx("Adding setting " + n + "(" + s + ")");
    return _imp->_settings.insert(make_pair(n, s));
}

unsigned long Bot::remove_setting(std::string n)
{
    Context ctx("Removing setting " + n);
    return _imp->_settings.erase(n);
}

void Bot::remove_setting(Bot::SettingsIterator it)
{
    Context ctx("Removing setting " + it->first);
    _imp->_settings.erase(it.underlying_iterator<Implementation<Bot>::SettingsMap::iterator>());
}

const ISupport* Bot::supported() const
{
    return &_imp->_supported;
}
