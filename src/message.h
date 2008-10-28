#ifndef message_h
#define message_h

#include <vector>
#include <string>
#include <tr1/memory>

namespace eir {
    class Bot;

    struct Message {
        Bot *bot;
        std::string source;
        std::string command;
        std::string destination;
        std::vector<std::string> args;

        Message(Bot *b, std::string s, std::string c, std::string d)
            : bot(b), source(s), command(c), destination(d)
        { }
        Message(Bot *b) : bot(b) { }
    };
}

#endif
