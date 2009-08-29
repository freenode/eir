#include "bot.h"
#include "command.h"
#include "exceptions.h"
#include "handler.h"

#include "server.h"

#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/tokeniser.hh>

#include <fstream>

#include "string_util.h"
#include <cstring>

using namespace eir;

using namespace std::tr1::placeholders;
using namespace paludis;

template class paludis::WrappedForwardIterator<Bot::ClientIteratorTag, const Client::ptr>;
template class paludis::WrappedForwardIterator<Bot::ChannelIteratorTag, const Channel::ptr>;
template class paludis::WrappedForwardIterator<Bot::SettingsIteratorTag, const std::pair<const std::string, Value> >;

namespace paludis
{
    struct cistringcompare
    {
        bool operator() (std::string lhs, std::string rhs)
        {
            return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
        }
    };


    template <>
    struct Implementation<BotManager>
    {
        typedef std::map<std::string, Bot*> BotMap;
        BotMap bots;
    };

    template <>
    struct Implementation<Bot> : public CommandHandlerBase<Implementation<Bot> >
    {
        typedef std::map<std::string, Client::ptr, cistringcompare> ClientMap;
        typedef std::map<std::string, Channel::ptr, cistringcompare> ChannelMap;
        typedef std::map<std::string, Value> SettingsMap;

        Bot *bot;

        std::string _name;

        std::tr1::shared_ptr<Server> _server;
        std::string _host, _port, _nick, _pass;

        Client::ptr _me;

        ClientMap _clients;
        ChannelMap _channels;
        SettingsMap _settings;

        bool _connected;

        ISupport _supported;

        void handle_message(std::string);

        CommandHolder set_handler;
        void handle_set(const Message *);

        void connect(std::string host, std::string port, std::string nick, std::string pass)
        {
            _server.reset(new Server(std::tr1::bind(&Implementation<Bot>::handle_message, this, _1)));
            _host = host;
            _port = port;
            _nick = nick;
            _pass = pass;
        }

        void set_server(const Message *m);

        std::string config_filename;
        CommandHolder rehash_handler;
        void load_config(std::tr1::function<void(std::string)>, bool cold = false);
        void rehash(const Message *m);

        Implementation(Bot *b, std::string n)
            : bot(b), _name(n), _connected(false), _supported(b)
        {
            config_filename = _name + ".conf";
            set_handler = add_handler(filter_command_privilege("set", "admin").from_bot(bot).or_config(),
                                      &Implementation<Bot>::handle_set);
            rehash_handler = add_handler(filter_command_privilege("rehash", "admin").from_bot(bot),
                                         &Implementation<Bot>::rehash);
        }
    };
}

#include <iostream>

static void print_cerr(std::string s)
{
    std::cerr << s << std::endl;
}

Bot::Bot(std::string botname)
    : PrivateImplementationPattern<Bot>(new Implementation<Bot> (this, botname))
{
    Implementation<BotManager>::BotMap::iterator it = BotManager::get_instance()->_imp->bots.find(botname);
    if (it != BotManager::get_instance()->_imp->bots.end())
        throw InternalError("There's already a bot called " + botname);

    BotManager::get_instance()->_imp->bots.insert(make_pair(botname, this));

    _imp->load_config(print_cerr, true);

    dispatch_internal_message(this, "config_loaded");
}

void Bot::connect(std::string host, std::string port, std::string nick, std::string pass)
{
    _imp->connect(host, port, nick, pass);
}

Bot::~Bot()
{
    dispatch_internal_message(this, "shutting_down");
}

void Implementation<Bot>::set_server(const Message *m)
{
    if (m->args.size() < 3)
        throw ConfigurationError("server needs three arguments.");

    std::string pass = m->args.size() > 3 ? m->args[3] : "";

    connect(m->args[0], m->args[1], m->args[2], pass);
}

void Implementation<Bot>::load_config(std::tr1::function<void(std::string)> reply_func, bool cold /* = false */)
{
    CommandHolder server_id;

    if (cold)
        server_id = add_handler(filter_command("server").from_bot(bot).source_type(sourceinfo::ConfigFile),
                                &Implementation<Bot>::set_server);

    std::ifstream fs(config_filename.c_str());
    std::string line;

    while(std::getline(fs, line))
    {
        std::list<std::string> tokens;
        paludis::tokenise_whitespace_quoted(line, std::back_inserter(tokens));

        if(tokens.empty())
            continue;

        Message m(bot, *tokens.begin());

        tokens.pop_front();
        std::copy(tokens.begin(), tokens.end(), std::back_inserter(m.args));

        if (cold)
            m.source.reply_func = reply_func;

        m.source.error_func = reply_func;

        m.source.type = sourceinfo::ConfigFile;

        m.raw = line;

        CommandRegistry::get_instance()->dispatch(&m, true);
    }
}

void Implementation<Bot>::rehash(const Message *m)
{
    Logger::get_instance()->Log(bot, m->source.client, Logger::Command, "REHASH");
    Logger::get_instance()->Log(bot, m->source.client, Logger::Admin, "Reloading config file");

    dispatch_internal_message(bot, "clear_lists");

    load_config(m->source.reply_func);

    dispatch_internal_message(bot, "recalculate_privileges");

    m->source.reply("Done.");
}

static void notice_to(Bot *b, std::string dest, std::string text)
{
    b->send("NOTICE " + dest + " :" + text);
}

void Implementation<Bot>::handle_message(std::string line)
{
    Context c("Parsing message " + line);

    Message m(bot);
    std::string::size_type p1, p2;
    std::string command;

    std::string::iterator e = line.end();
    if (*--e == '\n')
        line.erase(e);
    e = line.end();
    if (*--e == '\r')
        line.erase(e);

    m.raw = line;

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
        ClientMap::iterator c = _clients.find(nick);
        if (c != _clients.end())
            m.source.client = c->second;

        m.source.name = nick;
    }
    else
    {
        m.source.name = m.source.raw;
    }

    p2 = line.find(' ', p1);
    command = line.substr(p1, p2 - p1);

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

    m.source.error_func = m.source.reply_func;

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

    m.command = "server_incoming";
    m.source.type = sourceinfo::Internal;
    CommandRegistry::get_instance()->dispatch(&m);
    Logger::get_instance()->Log(bot, m.source.client, Logger::Raw, "<-- " + m.raw);
    m.command = command;
    m.source.type = sourceinfo::RawIrc;
    CommandRegistry::get_instance()->dispatch(&m);
}

void Implementation<Bot>::handle_set(const Message *m)
{
    if (m->bot != bot)
        return;

    if (m->args.size() < 2)
    {
        m->source.error("Not enough parameters to SET -- need two");
        return;
    }

    _settings[m->args[0]] = m->args[1];

    if (m->source.client)
        Logger::get_instance()->Log(bot, m->source.client, Logger::Command,
                                    "SET " + m->args[0] + " = " + m->args[1]);
    Logger::get_instance()->Log(bot, m->source.client, Logger::Admin,
                                "Set " + m->args[0] + " to " + m->args[1]);
}

const std::string& Bot::nick() const
{
    return _imp->_nick;
}

const std::string& Bot::name() const
{
    return _imp->_name;
}

const Client::ptr Bot::me() const
{
    return _imp->_me;
}

bool Bot::connected() const
{
    return _imp->_connected;
}

void Bot::disconnect(std::string reason)
{
    if (_imp->_server)
        _imp->_server->disconnect(reason);

    _imp->_connected = false;
}

void Bot::run()
{
    if ( ! _imp->_server)
        throw ConfigurationError("No server specified");

    _imp->_server->connect(_imp->_host, _imp->_port);

    _imp->_connected = true;

    Message m(this, "on_connect");
    CommandRegistry::get_instance()->dispatch(&m);

    if (_imp->_pass.length() > 0)
        send("PASS " + _imp->_pass);

    send("NICK " + _imp->_nick);
    send("USER eir * * :eir version 0.0.1");

    _imp->_server->run();
}

void Bot::send(std::string line)
{
    if (!_imp->_connected || !_imp->_server)
        throw NotConnectedException();

    Logger::get_instance()->Log(this, NULL, Logger::Raw, "--> " + line);

    _imp->_server->send(line);
}

// Client stuff

Bot::ClientIterator Bot::begin_clients()
{
    return second_iterator(_imp->_clients.begin());
}

Bot::ClientIterator Bot::end_clients()
{
    return second_iterator(_imp->_clients.end());
}

Bot::ClientIterator Bot::find_client_it(std::string nick)
{
    return second_iterator(_imp->_clients.find(nick));
}

Client::ptr Bot::find_client(std::string nick)
{
    Implementation<Bot>::ClientMap::iterator it = _imp->_clients.find(nick);
    if (it == _imp->_clients.end())
        return Client::ptr();
    return it->second;
}

std::pair<Bot::ClientIterator, bool> Bot::add_client(Client::ptr c)
{
    Context ctx("Adding client " + c->nick());

    // Bit of a hack this...
    if (!_imp->_me && c->nick() == nick())
        _imp->_me = c;

    std::pair<Implementation<Bot>::ClientMap::iterator, bool> res = _imp->_clients.insert(make_pair(c->nick(), c));
    if (res.second)
    {
        Message m(this, "new_client", sourceinfo::Internal, c);
        CommandRegistry::get_instance()->dispatch(&m);
    }

    return std::make_pair(ClientIterator(second_iterator(res.first)), res.second);
}

unsigned long Bot::remove_client(Client::ptr c)
{
    Context ctx("Removing client " + c->nick());

    Message m(this, "client_remove", sourceinfo::Internal, c);
    CommandRegistry::get_instance()->dispatch(&m);

    return _imp->_clients.erase(c->nick());
}

// Channel stuff

Bot::ChannelIterator Bot::begin_channels()
{
    return second_iterator(_imp->_channels.begin());
}

Bot::ChannelIterator Bot::end_channels()
{
    return second_iterator(_imp->_channels.end());
}

Bot::ChannelIterator Bot::find_channel_it(std::string name)
{
    return second_iterator(_imp->_channels.find(name));
}

Channel::ptr Bot::find_channel(std::string name)
{
    std::string lowername = lowercase(name);
    Implementation<Bot>::ChannelMap::iterator it = _imp->_channels.find(lowername);
    if (it == _imp->_channels.end())
        return Channel::ptr();
    return it->second;
}

std::pair<Bot::ChannelIterator, bool> Bot::add_channel(Channel::ptr c)
{
    Context ctx("Adding channel " + c->name());
    std::pair<Implementation<Bot>::ChannelMap::iterator, bool> res = _imp->_channels.insert(make_pair(c->name(), c));
    return make_pair(second_iterator(res.first), res.second);
}

unsigned long Bot::remove_channel(Channel::ptr c)
{
    Context ctx("Removing channel " + c->name());
    return _imp->_channels.erase(c->name());
}

void Bot::remove_channel(Bot::ChannelIterator c)
{
    Context ctx("Removing channel " + (*c)->name());
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

Value Bot::get_setting(std::string name)
{
    SettingsIterator it = find_setting(name);
    if(it == end_settings())
        return "";
    return it->second;
}

Value Bot::get_setting_with_default(std::string name, std::string _default)
{
    SettingsIterator it = find_setting(name);
    if(it == end_settings())
        return _default;
    return it->second;
}

std::pair<Bot::SettingsIterator, bool> Bot::add_setting(std::string n, Value s)
{
    Context ctx("Adding setting " + n + "(" + stringify(s) + ")");
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

BotManager::BotManager() : PrivateImplementationPattern<BotManager>(new Implementation<BotManager>)
{
}

BotManager::~BotManager()
{
}

Bot *BotManager::find(std::string name)
{
    Implementation<BotManager>::BotMap::iterator it = _imp->bots.find(name);
    if (it != _imp->bots.end())
        return it->second;
    return 0;
}
