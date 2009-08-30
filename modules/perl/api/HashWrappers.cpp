#include "HashWrappers.h"

#include <map>

using namespace eir;
using namespace eir::perl;

BotClientHash::BotClientHash(Bot *b) : _bot(b) { }
Client *BotClientHash::FETCH(char *nick) { return _bot->find_client(nick).get(); }
bool BotClientHash::EXISTS(char *nick) { return _bot->find_client(nick); }
int BotClientHash::SCALAR() { return _bot->begin_clients() != _bot->end_clients(); }

const char* BotClientHash::FIRSTKEY()
{
    Bot::ClientIterator it = _bot->begin_clients();
    if (it == _bot->end_clients())
        return 0;
    else
        return (*it)->nick().c_str();
}

const char* BotClientHash::NEXTKEY(char *prevnick)
{
    Bot::ClientIterator it = _bot->find_client_it(prevnick);
    if (it == _bot->end_clients())
        return 0;
    ++it;
    if (it == _bot->end_clients())
        return 0;
    return (*it)->nick().c_str();
}


BotChannelHash::BotChannelHash(Bot *b) : _bot(b) { }
Channel *BotChannelHash::FETCH(char *name) { return _bot->find_channel(name).get(); }
bool BotChannelHash::EXISTS(char *name) { return _bot->find_channel(name); }
int BotChannelHash::SCALAR() { return _bot->begin_channels() != _bot->end_channels(); }

const char* BotChannelHash::FIRSTKEY()
{
    Bot::ChannelIterator it = _bot->begin_channels();
    if (it == _bot->end_channels())
        return 0;
    else
        return (*it)->name().c_str();
}

const char* BotChannelHash::NEXTKEY(char *prevname)
{
    Bot::ChannelIterator it = _bot->find_channel_it(prevname);
    if (it == _bot->end_channels())
        return 0;
    ++it;
    if (it == _bot->end_channels())
        return 0;
    return (*it)->name().c_str();
}


ClientMembershipHash::ClientMembershipHash(Client *c) : _client(c) { }
Membership *ClientMembershipHash::FETCH(char *name) { return _client->find_membership(name).get(); }
bool ClientMembershipHash::EXISTS(char *name) { return _client->find_membership(name); }
int ClientMembershipHash::SCALAR() { return _client->begin_channels() != _client->end_channels(); }

const char* ClientMembershipHash::FIRSTKEY()
{
    Client::ChannelIterator it = _client->begin_channels();
    if (it == _client->end_channels())
        return 0;
    else
        return (*it)->channel->name().c_str();
}

const char* ClientMembershipHash::NEXTKEY(char *prevname)
{
    Client::ChannelIterator it = _client->find_membership_it(prevname);
    if (it == _client->end_channels())
        return 0;
    ++it;
    if (it == _client->end_channels())
        return 0;
    return (*it)->channel->name().c_str();
}


ChannelMembershipHash::ChannelMembershipHash(Channel *c) : _channel(c) { }
Membership *ChannelMembershipHash::FETCH(char *name) { return _channel->find_member(name).get(); }
bool ChannelMembershipHash::EXISTS(char *name) { return _channel->find_member(name); }
int ChannelMembershipHash::SCALAR() { return _channel->begin_members() != _channel->end_members(); }

const char* ChannelMembershipHash::FIRSTKEY()
{
    Channel::MemberIterator it = _channel->begin_members();
    if (it == _channel->end_members())
        return 0;
    else
        return (*it)->client->nick().c_str();
}

const char* ChannelMembershipHash::NEXTKEY(char *prevname)
{
    Channel::MemberIterator it = _channel->find_member_it(prevname);
    if (it == _channel->end_members())
        return 0;
    ++it;
    if (it == _channel->end_members())
        return 0;
    return (*it)->client->nick().c_str();
}


