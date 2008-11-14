#ifndef message_h
#define message_h

#include <vector>
#include <string>
#include <tr1/memory>

#include "client.h"

namespace eir {
    class Bot;

    struct sourceinfo {
        // If non-zero, this message came from somewhere other than the server,
        // and the value tells us where.
        int special;

        // Possible values for the above
        enum {
            ConfigFileSource = 1,
            SystemConsoleSource,
            SignalSource,
            InternalSource
        };

        // If client is null, source was a server or doesn't share
        // any channels with us.
        Client::ptr client;
        std::string name;

        // if in_channel is true, channel is the name of said channel.
        // else it was in private.
        bool in_channel;
        std::string channel;

        // The raw source string that the server sent us
        std::string raw;

        // The raw destination string.
        std::string destination;
    };

    struct Message {
        Bot *bot;
        sourceinfo source;
        std::string command;
        std::vector<std::string> args;

        std::string raw;

        Message(Bot *b) : bot(b) { }
        Message(Bot *b, std::string c) : bot(b), command(c) { }
    };
}

#endif
