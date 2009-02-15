#ifndef bot_h
#define bot_h

#include <string>

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>

#include "supported.h"
#include "client.h"
#include "message.h"

namespace eir
{
    class Bot : public paludis::PrivateImplementationPattern<Bot>
    {
        public:
            Bot(std::string name);

            void connect(std::string host, std::string port, std::string nick, std::string pass);

            const std::string& nick() const;
            const std::string& name() const;
            const Client::ptr me() const;

            void run();

            void disconnect(std::string);

            bool connected() const;

            void send(std::string);

            struct ClientIteratorTag;
            typedef paludis::WrappedForwardIterator<ClientIteratorTag, Client::ptr const> ClientIterator;
            ClientIterator begin_clients();
            ClientIterator end_clients();
            Client::ptr find_client(std::string nick);
            std::pair<ClientIterator, bool> add_client(Client::ptr c);
            unsigned long remove_client(Client::ptr c);

            struct ChannelIteratorTag;
            typedef paludis::WrappedForwardIterator<ChannelIteratorTag, Channel::ptr const> ChannelIterator;
            ChannelIterator begin_channels();
            ChannelIterator end_channels();
            Channel::ptr find_channel(std::string name);
            std::pair<ChannelIterator, bool> add_channel(Channel::ptr c);
            unsigned long remove_channel(Channel::ptr c);
            void remove_channel(ChannelIterator c);

            struct SettingsIteratorTag;
            typedef paludis::WrappedForwardIterator<SettingsIteratorTag,
                                        const std::pair<const std::string, std::string> > SettingsIterator;
            SettingsIterator begin_settings();
            SettingsIterator end_settings();
            SettingsIterator find_setting(std::string name);
            std::string get_setting(std::string name);
            std::string get_setting_with_default(std::string name, std::string _default);
            std::pair<SettingsIterator, bool> add_setting(std::string n, std::string s);
            unsigned long remove_setting(std::string n);
            void remove_setting(SettingsIterator it);

            const ISupport *supported() const;

            ~Bot();
    };

    class BotManager : public paludis::InstantiationPolicy<BotManager,
                                                           paludis::instantiation_method::SingletonTag>,
                       public paludis::PrivateImplementationPattern<BotManager>
    {
        public:
            friend class Bot;

            Bot *find(std::string name);
            BotManager();
            ~BotManager();
    };

}

#endif
