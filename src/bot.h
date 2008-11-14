#ifndef bot_h
#define bot_h

#include <string>
#include <map>

#include <paludis/util/wrapped_forward_iterator.hh>

#include "server.h"
#include "client.h"
#include "message.h"

namespace eir {

    class Bot
    {
        public:
            Bot(std::string host, std::string port, std::string nick, std::string pass);

            const std::string& nick() { return _nick; }

            const Client::ptr me() { return _me; }

            void run();

            void send(std::string);

            typedef std::map<std::string, Client::ptr> ClientMap;
            typedef ClientMap::iterator ClientIterator;
            ClientIterator begin_clients() { return _clients.begin(); }
            ClientIterator end_clients() { return _clients.end(); }
            ClientIterator find_client(std::string nick) {
                return _clients.find(nick);
            }
            std::pair<ClientIterator, bool> add_client(Client::ptr c) {
                return _clients.insert(make_pair(c->nick(), c));
            }
            ClientMap::size_type remove_client(Client::ptr c) {
                return _clients.erase(c->nick());
            }

            typedef std::map<std::string, Channel::ptr> ChannelMap;
            typedef ChannelMap::iterator ChannelIterator;
            ChannelIterator begin_channels() { return _channels.begin(); }
            ChannelIterator end_channels() { return _channels.end(); }
            ChannelIterator find_channel(std::string name) {
                return _channels.find(name);
            }
            std::pair<ChannelIterator, bool> add_channel(Channel::ptr c) {
                return _channels.insert(make_pair(c->name(), c));
            }
            ChannelMap::size_type remove_channel(Channel::ptr c) {
                return _channels.erase(c->name());
            }
            void remove_channel(ChannelIterator c) {
                _channels.erase(c);
            }

            typedef std::map<std::string, std::string> SettingsMap;
            typedef SettingsMap::iterator SettingsIterator;
            SettingsIterator begin_settings() { return _settings.begin(); }
            SettingsIterator end_settings() { return _settings.end(); }
            SettingsIterator find_setting(std::string name) {
                return _settings.find(name);
            }
            std::pair<SettingsIterator, bool> add_setting(std::string n, std::string s) {
                return _settings.insert(make_pair(n, s));
            }
            SettingsMap::size_type remove_setting(std::string n) {
                return _settings.erase(n);
            }
            void remove_setting(SettingsIterator it) {
                _settings.erase(it);
            }

            Bot();
            ~Bot();

        private:
            Server _server;
            std::string _host, _port, _nick, _pass;

            Client::ptr _me;

            ClientMap _clients;
            ChannelMap _channels;
            SettingsMap _settings;

            void handle_message(std::string);

            void _init_me(const Message *);
    };

}

#endif
