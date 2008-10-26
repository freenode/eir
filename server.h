#ifndef server_h
#define server_h

#include <tr1/functional>

#include "pimpl.h"

namespace eir
{
    class Server : private util::Pimpl<Server>
    {
        public:
            typedef std::tr1::function<void(std::string)> Handler;
            Server(const Handler&);
            ~Server();

            void connect(std::string host, std::string port);

            void send(std::string);

            void disconnect();

            void run();

        private:
            Server();
            Server (const Server &);
    };
}

#endif
