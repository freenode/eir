#ifndef message_h
#define message_h

#include <vector>
#include <string>
#include <tr1/memory>
#include <tr1/functional>

#include "client.h"

namespace eir {
    class Bot;

    struct sourceinfo {
        // If non-zero, this message came from somewhere other than the server,
        // and the value tells us where.
        enum {
            RawIrc        = 0x01,
            ConfigFile    = 0x02,
            SystemConsole = 0x04,
            Signal        = 0x08,
            Internal      = 0x10,
            IrcCommand    = 0x20,
            Any           = 0xff
        };
        unsigned int type;

        // If client is null, source was a server or doesn't share
        // any channels with us.
        Client::ptr client;
        std::string name;

        // The raw source string that the server sent us
        std::string raw;

        // The raw destination string.
        std::string destination;

        // Function to send a reply to this.
        std::tr1::function<void(std::string)> reply_func;
        void reply(std::string text) const {
            if(reply_func) reply_func(text);
        }

        sourceinfo(unsigned int t, Client::ptr c)
            : type(t), client(c), name(c->nick())
        { }
        sourceinfo() { };
    };

    struct Message {
        Bot *bot;
        sourceinfo source;
        std::string command;
        std::vector<std::string> args;

        std::string raw;

        Message(Bot *b) : bot(b) { }
        Message(Bot *b, std::string c) : bot(b), command(c) { }
        Message(Bot *b, std::string cmd, unsigned int t, Client::ptr cl)
            : bot(b), source(t, cl), command(cmd)
        { }
    };
}

#endif
