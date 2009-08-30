#ifndef hash_wrappers_h
#define hash_wrappers_h

#include "bot.h"

namespace eir
{
    namespace perl
    {
        class BotClientHash
        {
            Bot *_bot;

            public:
                BotClientHash(Bot *b)
                    : _bot(b)
                {
                }

                Client *FETCH(char *nick)
                {
                    return _bot->find_client(nick).get();
                }

                bool EXISTS(char *nick)
                {
                    return _bot->find_client(nick);
                }

                const char* FIRSTKEY()
                {
                    Bot::ClientIterator it = _bot->begin_clients();
                    if (it == _bot->end_clients())
                        return 0;
                    else
                        return (*it)->nick().c_str();
                }

                const char* NEXTKEY(char *prevnick)
                {
                    Bot::ClientIterator it = _bot->find_client_it(prevnick);
                    if (it == _bot->end_clients())
                        return 0;
                    ++it;
                    if (it == _bot->end_clients())
                        return 0;
                    return (*it)->nick().c_str();
                }

                int SCALAR()
                {
                    return _bot->begin_clients() != _bot->end_clients();
                }
        };
    }
}

#endif
