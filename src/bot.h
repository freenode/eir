#ifndef bot_h
#define bot_h

#include <string>

#include "server.h"
#include "command.h"

namespace eir {

    class Bot
    {
        public:
            Bot(std::string host, std::string port, std::string nick, std::string pass);

            Dispatcher *dispatcher() { return &_dispatcher; }

            void run();

            Bot();
            ~Bot();

        private:
            Dispatcher _dispatcher;
            Server _server;
            std::string _host, _port, _nick, _pass;

            void handle_message(std::string);
    };

}

#endif
