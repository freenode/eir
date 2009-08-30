#ifndef hash_wrappers_h
#define hash_wrappers_h

#include "bot.h"

namespace eir
{
    namespace perl
    {

        class BotChannelHash
        {
            Bot *_bot;
            public:
                BotChannelHash(Bot *b);
                Channel *FETCH(char *nick);
                bool EXISTS(char *nick);
                const char* FIRSTKEY();
                const char* NEXTKEY(char *prevnick);
                int SCALAR();
        };

        class BotClientHash
        {
            Bot *_bot;
            public:
                BotClientHash(Bot *b);
                Client *FETCH(char *nick);
                bool EXISTS(char *nick);
                const char* FIRSTKEY();
                const char* NEXTKEY(char *prevnick);
                int SCALAR();
        };

        class ClientMembershipHash
        {
            Client *_client;
            public:
                ClientMembershipHash(Client *b);
                Membership *FETCH(char *nick);
                bool EXISTS(char *nick);
                const char* FIRSTKEY();
                const char* NEXTKEY(char *prevnick);
                int SCALAR();
        };

        class ChannelMembershipHash
        {
            Channel *_channel;
            public:
                ChannelMembershipHash(Channel *b);
                Membership *FETCH(char *nick);
                bool EXISTS(char *nick);
                const char* FIRSTKEY();
                const char* NEXTKEY(char *prevnick);
                int SCALAR();
        };

    }
}

#endif
