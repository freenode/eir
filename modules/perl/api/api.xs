#include "stupid_perl_workarounds.h"

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "eir.h"

#include "HashWrappers.h"

using namespace eir;
using namespace eir::perl;
using std::string;

#include "util/type_maps.h"


MODULE = Eir            PACKAGE = Eir

Bot *
find_bot(name)
    char *name
CODE:
    RETVAL = BotManager::get_instance()->find(name);
OUTPUT:
    RETVAL


MODULE = Eir            PACKAGE = Eir::Bot

void
Bot::send(char* name)

string
Bot::nick()

string
Bot::name()


MODULE = Eir            PACKAGE = Eir::Client

string
Client::nick()

string
Client::user()

string
Client::host()

string
Client::nuh()

ClientMembershipHash *
Client::channels()
CODE:
    RETVAL = new ClientMembershipHash(THIS);
OUTPUT:
    RETVAL


MODULE = Eir            PACKAGE = Eir::Channel

string
Channel::name()

ChannelMembershipHash *
Channel::members()
CODE:
    RETVAL = new ChannelMembershipHash(THIS);
OUTPUT:
    RETVAL


MODULE = Eir            PACKAGE = Eir::Membership

Client *
Membership::client()
CODE:
    RETVAL = THIS->client.get();
OUTPUT:
    RETVAL

Channel *
Membership::channel()
CODE:
    RETVAL = THIS->channel.get();
OUTPUT:
    RETVAL

string
Membership::modes()
CODE:
    RETVAL = THIS->modes;
OUTPUT:
    RETVAL

int
Membership::has_mode(char m)


MODULE = Eir            PACKAGE = Eir::Internal::BotClientHash

BotClientHash *
TIEHASH(char *classname, Bot* bot)
CODE:
    RETVAL = new BotClientHash(bot);
OUTPUT:
    RETVAL

Client *
BotClientHash::FETCH(char *nick)

void
BotClientHash::STORE(char *nick, SV *foo)
CODE:
    Perl_croak("Tried to modify a read-only tied hash Bot::Clients");

void
BotClientHash::DELETE(char *nick)
CODE:
    Perl_croak("Tried to modify a read-only tied hash Bot::Clients");

int
BotClientHash::EXISTS(char *nick)

const char *
BotClientHash::FIRSTKEY()

const char *
BotClientHash::NEXTKEY(char *prev)

void
BotClientHash::DESTROY()

int
BotClientHash::SCALAR()


MODULE = Eir            PACKAGE = Eir::Internal::BotChannelHash

BotChannelHash *
TIEHASH(char *classname, Bot* bot)
CODE:
    RETVAL = new BotChannelHash(bot);
OUTPUT:
    RETVAL

Channel *
BotChannelHash::FETCH(char *nick)

void
BotChannelHash::STORE(char *nick, SV *foo)
CODE:
    Perl_croak("Tried to modify a read-only tied hash Bot::Channels");

void
BotChannelHash::DELETE(char *nick)
CODE:
    Perl_croak("Tried to modify a read-only tied hash Bot::Channels");

int
BotChannelHash::EXISTS(char *nick)

const char *
BotChannelHash::FIRSTKEY()

const char *
BotChannelHash::NEXTKEY(char *prev)

void
BotChannelHash::DESTROY()

int
BotChannelHash::SCALAR()


MODULE = Eir            PACKAGE = Eir::Internal::ClientMembershipHash

ClientMembershipHash *
TIEHASH(char *classname, Client* client)
CODE:
    RETVAL = new ClientMembershipHash(client);
OUTPUT:
    RETVAL

Membership *
ClientMembershipHash::FETCH(char *name)

void
ClientMembershipHash::STORE(char *name, SV *foo)
CODE:
    Perl_croak("Tried to modify a read-only tied hash Bot::Channels");

void
ClientMembershipHash::DELETE(char *name)
CODE:
    Perl_croak("Tried to modify a read-only tied hash Bot::Channels");

int
ClientMembershipHash::EXISTS(char *name)

const char *
ClientMembershipHash::FIRSTKEY()

const char *
ClientMembershipHash::NEXTKEY(char *prev)

void
ClientMembershipHash::DESTROY()

int
ClientMembershipHash::SCALAR()


MODULE = Eir            PACKAGE = Eir::Internal::ChannelMembershipHash

ChannelMembershipHash *
TIEHASH(char *classname, Channel* channel)
CODE:
    RETVAL = new ChannelMembershipHash(channel);
OUTPUT:
    RETVAL

Membership *
ChannelMembershipHash::FETCH(char *name)

void
ChannelMembershipHash::STORE(char *name, SV *foo)
CODE:
    Perl_croak("Tried to modify a read-only tied hash Bot::Channels");

void
ChannelMembershipHash::DELETE(char *name)
CODE:
    Perl_croak("Tried to modify a read-only tied hash Bot::Channels");

int
ChannelMembershipHash::EXISTS(char *name)

const char *
ChannelMembershipHash::FIRSTKEY()

const char *
ChannelMembershipHash::NEXTKEY(char *prev)

void
ChannelMembershipHash::DESTROY()

int
ChannelMembershipHash::SCALAR()

