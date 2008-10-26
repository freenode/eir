#ifndef message_h
#define message_h

#include <vector>
#include <string>
#include <tr1/memory>

namespace eir {
    struct Message {
        std::string source;
        std::string destination;
        std::string command;
        std::vector<std::string> args;

        Message(std::string s, std::string d, std::string c)
            : source(s), destination(d), command(c)
        { }
        Message() { }
    };
}

#endif
