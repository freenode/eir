#ifndef bot_h
#define bot_h

#include <string>

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/private_implementation_pattern.hh>

#include "server.h"
#include "client.h"
#include "message.h"

namespace eir
{
    class Bot : public paludis::PrivateImplementationPattern<Bot>
    {
        public:
            Bot(std::string host, std::string port, std::string nick, std::string pass);

            const std::string& nick();

            const Client::ptr me();

            void run();

            void send(std::string);

            struct ClientIteratorTag;
            typedef paludis::WrappedForwardIterator<ClientIteratorTag,
                                        const std::pair<const std::string, Client::ptr> > ClientIterator;
            ClientIterator begin_clients();
            ClientIterator end_clients();
            ClientIterator find_client(std::string nick);
            std::pair<ClientIterator, bool> add_client(Client::ptr c);
            unsigned long remove_client(Client::ptr c);

            struct ChannelIteratorTag;
            typedef paludis::WrappedForwardIterator<ChannelIteratorTag,
                                        const std::pair<const std::string, Channel::ptr> > ChannelIterator;
            ChannelIterator begin_channels();
            ChannelIterator end_channels();
            ChannelIterator find_channel(std::string name);
            std::pair<ChannelIterator, bool> add_channel(Channel::ptr c);
            unsigned long remove_channel(Channel::ptr c);
            void remove_channel(ChannelIterator c);

            struct SettingsIteratorTag;
            typedef paludis::WrappedForwardIterator<SettingsIteratorTag,
                                        const std::pair<const std::string, std::string> > SettingsIterator;
            SettingsIterator begin_settings();
            SettingsIterator end_settings();
            SettingsIterator find_setting(std::string name);
            std::pair<SettingsIterator, bool> add_setting(std::string n, std::string s);
            unsigned long remove_setting(std::string n);
            void remove_setting(SettingsIterator it);

            Bot();
            ~Bot();
   };
}

#endif
