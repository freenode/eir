#ifndef server_h
#define server_h

#include <tr1/functional>
#include <paludis/util/private_implementation_pattern.hh>

namespace eir
{
    class Server : private paludis::PrivateImplementationPattern<Server>
    {
        public:
            typedef std::tr1::function<void(std::string)> Handler;
            Server(const Handler&);
            ~Server();

            void connect(std::string host, std::string port);

            void send(std::string);

            void purge();

            void disconnect();

            void run();

        private:
            Server();
            Server (const Server &);
    };
}

#endif
