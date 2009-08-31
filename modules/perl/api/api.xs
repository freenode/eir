#include "stupid_perl_workarounds.h"

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "eir.h"

#include "HashWrappers.h"
#include "perl_helpers.hh"

#include <functional>
using namespace std::placeholders;


using namespace eir;
using namespace eir::perl;
using std::string;

// Include order matters here. type_maps.h MUST come after HashWrappers.h
// and perl_helpers.hh, after the above using directives, and before
// call_perl.hh.
#include "util/type_maps.h"

#include "util/call_perl.hh"

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

int
Client::has_privilege(const char *priv)
CODE:
    RETVAL = THIS->privs().has_privilege(priv);
OUTPUT:
    RETVAL


MODULE = Eir            PACKAGE = Eir::Channel

string
Channel::name()

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


MODULE = Eir            PACKAGE = Eir::Filter

Filter *
do_new(const char *classname)
CODE:
    RETVAL = new Filter();
OUTPUT:
    RETVAL

Filter *
Filter::is_command(const char *cmd)
CODE:
    THIS->is_command(cmd);
    RETVAL = THIS;
OUTPUT:
    RETVAL

Filter *
Filter::source_type(int type)
CODE:
    THIS->source_type(type);
    RETVAL = THIS;
OUTPUT:
    RETVAL

Filter *
Filter::source_named(const char *src)
CODE:
    THIS->source_named(src);
    RETVAL = THIS;
OUTPUT:
    RETVAL

Filter *
Filter::from_bot(Bot *b)
CODE:
    THIS->from_bot(b);
    RETVAL = THIS;
OUTPUT:
    RETVAL

Filter *
Filter::in_private()
CODE:
    THIS->in_private();
    RETVAL = THIS;
OUTPUT:
    RETVAL

Filter *
Filter::in_channel(const char *chan)
CODE:
    THIS->in_channel(chan);
    RETVAL = THIS;
OUTPUT:
    RETVAL

Filter *
Filter::requires_privilege(const char *priv)
CODE:
    THIS->requires_privilege(priv);
    RETVAL = THIS;
OUTPUT:
    RETVAL

Filter *
Filter::or_config()
CODE:
    THIS->or_config();
    RETVAL = THIS;
OUTPUT:
    RETVAL

int
Filter::match(Message *m)


MODULE = Eir            PACKAGE = Eir::Message

Bot *
Message::bot()
CODE:
    RETVAL = THIS->bot;
OUTPUT:
    RETVAL

string
Message::command()
CODE:
    RETVAL = THIS->command;
OUTPUT:
    RETVAL

string
Message::raw()
CODE:
    RETVAL = THIS->raw;
OUTPUT:
    RETVAL

SV *
Message::args()
CODE:
    AV *ret = newAV();
    for (auto it = THIS->args.begin(); it != THIS->args.end(); ++it)
        av_push(ret, newSVpv(it->c_str(), 0));
    RETVAL = newRV_noinc((SV*)ret);
OUTPUT:
    RETVAL

SV *
Message::source()
CODE:
    HV *ret = newHV();
    hv_store(ret, "type", 4, newSViv(THIS->source.type), 0);
    SV *client = newSV(0);
    sv_setref_pv(client, "Eir::Client", THIS->source.client.get());
    hv_store(ret, "client", 6, client, 0);
    hv_store(ret, "name", 4, newSVpv(THIS->source.name.c_str(), 0), 0);
    hv_store(ret, "raw", 3, newSVpv(THIS->source.raw.c_str(), 0), 0);
    hv_store(ret, "destination", 11, newSVpv(THIS->source.destination.c_str(), 0), 0);

    RETVAL = newRV_noinc((SV*)ret);
OUTPUT:
    RETVAL


void
Message::reply(const char *str)
CODE:
    THIS->source.reply(str);


MODULE = Eir            PACKAGE = Eir::Internal::PerlHolder

void
PerlHolder::DESTROY()

MODULE = Eir            PACKAGE = Eir::CommandRegistry

PerlHolder *
add_handler(Filter *filter, SV *func)
CODE:
    CommandRegistry *reg = CommandRegistry::get_instance();
    CommandRegistry::id id = reg->add_handler(
                                *filter,
                                std::bind(call_perl<PerlContext::Void, SV*, const Message *>, func, _1));
    RETVAL = new PerlCommandHolder(id, func);
OUTPUT:
    RETVAL
