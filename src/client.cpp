#include "client.h"

#include <map>
#include <set>

#include <paludis/util/wrapped_forward_iterator-impl.hh>

using namespace eir;

namespace eir
{
    template <>
    struct Implementation<Client>
    {
        std::string nick, user, host;

        std::map<std::string, std::string> attributes;
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

const std::string& Client::attr(const std::string &name) const
{
    return _imp->attributes[name];
}

void Client::set_attr(const std::string &name, const std::string &value)
{
    _imp->attributes[name] = value;
}

namespace
{
    struct ClientComparator
    {
        int operator()(Client *l, Client *r)
        {
            return l->nick().compare(r->nick());
        }
    };
}

namespace eir
{
    template <>
    struct Implementation<Channel>
    {
        std::string name;

        typedef std::set<Client *, ClientComparator>::iterator MemberIterator;
        std::set<Client *, ClientComparator> members;
    };
}

Channel::MemberIterator Channel::begin_members()
{
    return Channel::MemberIterator(_imp->members.begin());
}

Channel::MemberIterator Channel::end_members()
{
    return Channel::MemberIterator(_imp->members.end());
}

void Channel::add_member(struct Client *c)
{
    _imp->members.insert(c);
}

bool Channel::remove_member(struct Client *c)
{
    return _imp->members.erase(c) != 0;
}


