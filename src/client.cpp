#include "client.h"
#include "exceptions.h"
#include "bot.h"
#include "handler.h"

#include <map>
#include <set>

#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

#include "string_util.h"

using namespace eir;

template class paludis::WrappedForwardIterator<eir::Client::ChannelIteratorTag, eir::Membership::ptr const>;
template class paludis::WrappedForwardIterator<eir::Channel::MemberIteratorTag, eir::Membership::ptr const>;
template class paludis::WrappedForwardIterator<eir::Client::AttributeIteratorTag, std::pair<const std::string, eir::Value> >;
template class paludis::WrappedForwardIterator<eir::Channel::AttributeIteratorTag, std::pair<const std::string, eir::Value> >;

namespace paludis
{
    template <>
    struct Implementation<Client>
    {
        Bot *bot;

        std::string nick, user, host, account;

        std::map<std::string, Value> attributes;

        std::map<std::string, Membership::ptr> channels;

        PrivilegeSet privs;

        mutable std::string nuh_cache;
        mutable bool nuh_cached;

        Implementation(Bot *b, std::string n, std::string u, std::string h)
            : bot(b), nick(n), user(u), host(h), nuh_cached(false)
        { }
    };
}

const std::string& Client::nick() const { return _imp->nick; }
const std::string& Client::user() const { return _imp->user; }
const std::string& Client::host() const { return _imp->host; }
const std::string& Client::account() const { return _imp->account; }

const std::string& Client::nuh() const
{
    if(_imp->nuh_cached)
        return _imp->nuh_cache;

    _imp->nuh_cache = _imp->nick + "!" + _imp->user + "@" + _imp->host;
    _imp->nuh_cached = true;

    return _imp->nuh_cache;
}

void Client::change_nick(std::string newnick)
{
    // The correct logic for this is somewhat non-trivial, due to the various
    // objects that index client objects by nickname. These all need to be
    // properly updated.
    //
    // So, three stages. Remove the old nickname from all such indices, then
    // change the nickname, then re-add the new nickname.
    for (auto it = _imp->channels.begin(); it != _imp->channels.end(); ++it)
        it->second->channel->remove_member(it->second);

    _imp->bot->remove_client(shared_from_this());

    _imp->nick = newnick;
    _imp->nuh_cached = false;

    _imp->bot->add_client(shared_from_this());

    for (auto it = _imp->channels.begin(); it != _imp->channels.end(); ++it)
        it->second->channel->add_member(it->second);
}

void Client::set_account(std::string accountname)
{
    // Protocol uses * to mean "no account name", so blank it out if that is found
    if (accountname == "*")
        accountname = "";

    // Don't invalidate privileges if nothing changed
    if (accountname == _imp->account)
        return;

    _imp->account = accountname;
    dispatch_internal_message(_imp->bot, "recalculate_privileges");
}

Client::AttributeIterator Client::attr_begin()
{
    return _imp->attributes.begin();
}

Client::AttributeIterator Client::attr_end()
{
    return _imp->attributes.end();
}

Value Client::attr(const std::string &name)
{
    return _imp->attributes[name];
}

void Client::set_attr(const std::string &name, const Value &value)
{
    _imp->attributes[name] = value;
}

Client::ChannelIterator Client::begin_channels()
{
    return second_iterator(_imp->channels.begin());
}

Client::ChannelIterator Client::end_channels()
{
    return second_iterator(_imp->channels.end());
}

Membership::ptr Client::find_membership(std::string chname)
{
    std::map<std::string, Membership::ptr>::iterator it = _imp->channels.find(chname);
    if (it == _imp->channels.end())
        return Membership::ptr();
    return it->second;
}

Client::ChannelIterator Client::find_membership_it(std::string chname)
{
    return second_iterator(_imp->channels.find(chname));
}

Membership::ptr Client::join_chan(Channel::ptr c)
{
    Context ctx("Adding client " + _imp->nick + " to channel " + c->name());
    Membership::ptr m;

    if (m = find_membership(c->name()))
        return m;

    m.reset(new Membership(shared_from_this(), c));

    if(c->add_member(m))
        _imp->channels.insert(make_pair(c->name(), m));

    return m;
}

void Client::leave_chan(Channel::ptr c)
{
    Context ctx("Removing client " + _imp->nick + "from channel " + c->name());
    Membership::ptr m = find_membership(c->name());
    if (m)
        leave_chan(m);
}

void Client::leave_chan(Membership::ptr m)
{
    if (m->client.get() != this)
        return;

    m->channel->remove_member(m);
    _imp->channels.erase(m->channel->name());
}

PrivilegeSet& Client::privs()
{
    return _imp->privs;
}

Client::Client(Bot *b, std::string n, std::string u, std::string h)
    : paludis::PrivateImplementationPattern<Client>(new paludis::Implementation<Client>(b, n, u, h))
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

        std::map<std::string, Value> attributes;

        typedef std::map<std::string, Membership::ptr>::iterator MemberIterator;
        std::map<std::string, Membership::ptr> members;

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
    return second_iterator(_imp->members.begin());
}

Channel::MemberIterator Channel::end_members()
{
    return second_iterator(_imp->members.end());
}

Channel::MemberIterator Channel::find_member_it(std::string nick)
{
    return second_iterator(_imp->members.find(nick));
}

MembershipPtr Channel::find_member(std::string nick)
{
    std::map<std::string, Membership::ptr>::iterator it = _imp->members.find(nick);
    if (it == _imp->members.end())
        return Membership::ptr();
    return it->second;
}

bool Channel::add_member(Membership::ptr m)
{
    return _imp->members.insert(std::make_pair(m->client->nick(), m)).second;
}

bool Channel::remove_member(Membership::ptr m)
{
    return _imp->members.erase(m->client->nick()) != 0;
}

Channel::AttributeIterator Channel::attr_begin()
{
    return _imp->attributes.begin();
}

Channel::AttributeIterator Channel::attr_end()
{
    return _imp->attributes.end();
}

Value Channel::attr(const std::string &name)
{
    return _imp->attributes[name];
}

void Channel::set_attr(const std::string &name, const Value &value)
{
    _imp->attributes[name] = value;
}

Channel::Channel(std::string n)
    : paludis::PrivateImplementationPattern<Channel>(new paludis::Implementation<Channel>(n))
{
}

Channel::~Channel()
{
}
