#include "server.h"
#include "exceptions.h"

#include <iostream>
#include <boost/asio.hpp>

#include <paludis/util/private_implementation_pattern-impl.hh>

#include <queue>
#include <cstdlib>

using namespace eir;
using boost::asio::ip::tcp;

#include <iostream>

namespace paludis
{
    template<>
    struct Implementation<eir::Server> {
        Server::Handler _handler;

        std::queue<std::string> _send_queue;
        tcp::iostream _stream;

        int _last_sync_cookie;
        int _lines_since_sync;


        void received(std::string line)
        {
            if (line.find("PONG") != std::string::npos && 
                    std::atoi(line.substr(line.rfind(':')+1).c_str()) == _last_sync_cookie)
            {
                std::cerr << "Got sync pong " << _last_sync_cookie << std::endl;
                _lines_since_sync = 0;
                _last_sync_cookie = 0;
                maybe_send();
                return;
            }
            _handler(line);
        }

        void maybe_send()
        {
            while (_lines_since_sync < 4 && !_send_queue.empty())
            {
                std::cerr << "--> " << _send_queue.front();
                _stream << _send_queue.front();
                _send_queue.pop();
                _lines_since_sync++;
            }
            if (_lines_since_sync >= 4 && _last_sync_cookie == 0)
            {
                _last_sync_cookie = std::rand();
                std::cerr << "Sending sync ping " << _last_sync_cookie << std::endl;
                _stream << "PING :" << _last_sync_cookie << "\r\n";
            }
        }

        Implementation(const std::tr1::function<void(std::string)>& h) : _handler(h)
        {
        }

        ~Implementation()
        {
        }
    };
}

Server::Server(const Handler& handler)
    : paludis::PrivateImplementationPattern<Server>(new paludis::Implementation<Server>(handler))
{
}

Server::~Server()
{
}

void Server::connect(std::string host, std::string port)
{
    _imp->_stream.connect(host, port);
}

void Server::disconnect()
{
    _imp->_stream.close();
}

void Server::purge()
{
    while (! _imp->_send_queue.empty())
        _imp->_send_queue.pop();
}

void Server::send(std::string line)
{
    Context c("Sending line " + line);
    std::string::size_type p;

    p = line.rfind("\r\n");
    if(p != std::string::npos)
        line = line.substr(0, p);
    else
    {
        p = line.rfind("\n");
        if (p != std::string::npos)
            line = line.substr(0, p);
    }
    line += "\r\n";

    _imp->_send_queue.push(line);
    _imp->maybe_send();
}

void Server::run()
{
    Context c("In main message loop");

    std::string line;
    while(std::getline(_imp->_stream, line))
    {
        _imp->received(line);
    }
}
