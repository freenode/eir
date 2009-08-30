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

const char *
BotClientHash::DESTROY()

int
BotClientHash::SCALAR()
