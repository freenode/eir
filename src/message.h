#ifndef message_h
#define message_h

#include <vector>
#include <string>
#include <tr1/memory>

namespace eir {
    class Bot;

    struct sourceinfo {
        // If client is null, source was a server or doesn't share
        // any channels with us.
        Client *client;
        std::string name;

        // if in_channel is true, channel is the name of said channel.
        // else it was in private.
        bool in_channel;
        std::string channel;

        // The raw source string that the server sent us
        std::string raw;
    };

    struct Message {
        Bot *bot;
        sourceinfo source;
        std::string command;
        std::vector<std::string> args;

        Message(Bot *b) : bot(b) { }
        Message(Bot *b, std::string c) : bot(b), command(c) { }
    };
}

#endif
