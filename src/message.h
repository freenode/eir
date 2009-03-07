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
        std::tr1::function<void(std::string)> reply_func, error_func;
        void reply(std::string text) const {
            if(reply_func) reply_func(text);
        }
        void error(std::string text) const {
            if(error_func) error_func(text);
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

        Message(const Message& m, unsigned int type, std::string c)
            : bot(m.bot), source(m.source), command(c)
        { source.type = type; }
    };

    class Filter {
        enum {
            match_command = 1,
            match_bot = 2,
            match_privilege = 4,
            match_private = 8,
            match_in_channel = 16,
            match_source_type = 32,
            match_source_name = 64,
            match_config_overrides = 128
        };
        unsigned matches;
        std::string commandname, privilege, channel, source;
        Bot *bot;
        unsigned sourcetype;

        public:
            Filter();
            Filter& is_command(std::string);
            Filter& source_type(unsigned int);
            Filter& source_named(std::string);
            Filter& from_bot(Bot *);
            Filter& in_private();
            Filter& in_channel(std::string);
            Filter& requires_privilege(std::string);
            Filter& or_config();

            bool match(const Message *) const;
    };

    inline Filter filter_type(unsigned int type)
    { return Filter().source_type(type); }

    inline Filter filter_command(std::string cmd)
    { return Filter().is_command(cmd); }

    inline Filter filter_command_type(std::string cmd, unsigned int type)
    { return Filter().is_command(cmd).source_type(type); }

    inline Filter filter_command_privilege(std::string cmd, std::string priv)
    { return Filter().is_command(cmd).requires_privilege(priv); }

    inline Filter filter_bot(Bot *b)
    { return Filter().from_bot(b); }

    inline Filter filter() { return Filter(); }
}

#endif
