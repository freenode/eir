#include "server.h"

#include <iostream>
#include <boost/asio.hpp>

using namespace eir;
using boost::asio::ip::tcp;

namespace eir
{
    template<>
    struct Implementation<eir::Server> {
        Server::Handler _handler;

        tcp::iostream _stream;

        Implementation(const std::tr1::function<void(std::string)>& h) : _handler(h)
        {
        }

        ~Implementation()
        {
        }
    };
}

Server::Server(const Handler& handler)
    : util::Pimpl<Server>(new Implementation<Server>(handler))
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

void Server::send(std::string line)
{
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

    _imp->_stream << line;
}



void Server::run()
{
    std::string line;
    while(std::getline(_imp->_stream, line))
    {
        _imp->_handler(line);
    }
}
