#ifndef bot_h
#define bot_h

#include <string>

#include "server.h"

namespace eir {

    class Bot
    {
        public:
            Bot(std::string host, std::string port, std::string nick, std::string pass);

            void run();

            void send(std::string);

            Bot();
            ~Bot();

        private:
            Server _server;
            std::string _host, _port, _nick, _pass;

            void handle_message(std::string);
    };

}

#endif
