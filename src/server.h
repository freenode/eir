#ifndef server_h
#define server_h

#include <string>
#include <functional>
#include <paludis/util/private_implementation_pattern.hh>
#include <ctime>

namespace eir
{
    class Server : private paludis::PrivateImplementationPattern<Server>
    {
        public:
            typedef std::function<void(std::string)> Handler;
            Server(const Handler&);
            ~Server();

            void connect(std::string host, std::string port);

            void send(std::string);

            void purge();

            void disconnect(std::string message);

            void run();

            void set_throttle(int burst, int time, int num);

        private:
            Server();
            Server (const Server &);
    };
}

#endif
