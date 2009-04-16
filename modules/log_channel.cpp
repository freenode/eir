#include "eir.h"

using namespace eir;

struct ChannelLogger : public Module
{
    struct Destination : public LogDestination
    {
        std::string channel;

        void Log(Bot *b, Client *c, std::string text)
        {
            b->send("PRIVMSG " + channel + " :(" + c->nick() + ") " + text);
        }

        Destination(std::string ch)
            : channel(ch)
        { }
    };

    struct Backend : public LogBackend
    {
        LogDestination *create_destination(std::string channel)
        {
            return new Destination(channel);
        }
    };

    LogBackendHolder id;

    ChannelLogger()
    {
        id = Logger::get_instance()->register_backend("channel", new Backend);
    }
};

MODULE_CLASS(ChannelLogger)

