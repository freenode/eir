#include "eir.h"

#include <iostream>

using namespace eir;

struct StdErrLogger : public Module
{
    struct Destination : public LogDestination
    {
        void Log(Bot *b, Client *, std::string text)
        {
            std::string::size_type p = text.rfind("\r\n");
            if(p != std::string::npos)
                text = text.substr(0, p);
            else
            {
                p = text.rfind("\n");
                if (p != std::string::npos)
                    text = text.substr(0, p);
            }

            if (b)
                std::cerr << "[" << b->name() << "] ";
            std::cerr << text << std::endl;
        }
    };

    struct Backend : public LogBackend
    {
        LogDestination *create_destination(std::string)
        {
            return new Destination;
        }
    };

    LogBackendHolder id;

    StdErrLogger()
    {
        id = Logger::get_instance()->register_backend("stderr", new Backend);
    }
};

MODULE_CLASS(StdErrLogger)
