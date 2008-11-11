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

            void run();

            void send(std::string);

            typedef std::map<std::string, Client *> ClientMap;
            typedef ClientMap::iterator ClientIterator;
            ClientIterator begin_clients() { return _clients.begin(); }
            ClientIterator end_clients() { return _clients.end(); }
            ClientIterator find_client(std::string nick) {
                return _clients.find(nick);
            }

            typedef std::map<std::string, Channel *> ChannelMap;
            typedef ChannelMap::iterator ChannelIterator;
            ChannelIterator begin_channels() { return _channels.begin(); }
            ChannelIterator end_channels() { return _channels.end(); }
            ChannelIterator find_channel(std::string name) {
                return _channels.find(name);
            }

            Bot();
            ~Bot();

        private:
            Server _server;
            std::string _host, _port, _nick, _pass;

            ClientMap _clients;
            ChannelMap _channels;

            void handle_message(std::string);

            void _handle_join(const Message *);
            void _handle_part(const Message *);
            void _handle_quit(const Message *);
    };

}

#endif
