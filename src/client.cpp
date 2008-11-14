#include "client.h"

#include <map>
#include <set>

#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace eir;

namespace paludis
{
    template <>
    struct Implementation<Client>
    {
        std::string nick, user, host;

        std::map<std::string, std::string> attributes;

        std::set<Channel::ptr> channels;

        Implementation(std::string n, std::string u, std::string h)
            : nick(n), user(u), host(h)
        { }
    };
}

const std::string& Client::nick() const { return _imp->nick; }
const std::string& Client::user() const { return _imp->user; }
const std::string& Client::host() const { return _imp->host; }

Client::AttributeIterator Client::attr_begin()
{
    return _imp->attributes.begin();
}

Client::AttributeIterator Client::attr_end()
{
    return _imp->attributes.end();
}

std::string Client::attr(const std::string &name)
{
    return _imp->attributes[name];
}

void Client::set_attr(const std::string &name, const std::string &value)
{
    _imp->attributes[name] = value;
}

Client::ChannelIterator Client::begin_channels()
{
    return _imp->channels.begin();
}

Client::ChannelIterator Client::end_channels()
{
    return _imp->channels.end();
}

void Client::join_chan(Channel::ptr c)
{
    if(c->add_member(shared_from_this()))
        _imp->channels.insert(c);
}

void Client::leave_chan(Channel::ptr c)
{
    c->remove_member(shared_from_this());
    _imp->channels.erase(c);
}

Client::Client(std::string n, std::string u, std::string h)
    : paludis::PrivateImplementationPattern<Client>(new paludis::Implementation<Client>(n, u, h))
{
}

Client::~Client()
{
}

namespace
{
    struct ClientComparator
    {
        int operator()(Client::ptr l, Client::ptr r)
        {
            return l->nick().compare(r->nick());
        }
    };
}

namespace paludis
{
    template <>
    struct Implementation<Channel>
    {
        std::string name;

        typedef std::set<Client::ptr, ClientComparator>::iterator MemberIterator;
        std::set<Client::ptr, ClientComparator> members;

        Implementation(std::string n) : name(n)
        { }
    };
}

const std::string& Channel::name()
{
    return _imp->name;
}

Channel::MemberIterator Channel::begin_members()
{
    return Channel::MemberIterator(_imp->members.begin());
}

Channel::MemberIterator Channel::end_members()
{
    return Channel::MemberIterator(_imp->members.end());
}

bool Channel::add_member(Client::ptr c)
{
    return _imp->members.insert(c).second;
}

bool Channel::remove_member(Client::ptr c)
{
    return _imp->members.erase(c) != 0;
}

Channel::Channel(std::string n)
    : paludis::PrivateImplementationPattern<Channel>(new paludis::Implementation<Channel>(n))
{
}

Channel::~Channel()
{
}

template class paludis::WrappedForwardIterator<eir::Client::ChannelIteratorTag, std::tr1::shared_ptr<eir::Channel> const>;
