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


MODULE = Eir            PACKAGE = Eir::CommandRegistry

PerlHolder *
add_handler(Filter *filter, SV *func)
CODE:
    CommandRegistry *reg = CommandRegistry::get_instance();
    CommandRegistry::id id = reg->add_handler(
                                *filter,
                                std::bind(call_perl<PerlContext::Void, const char *, SV*, const Message *>,
                                            aTHX_ "Eir::Init::call_wrapper", func, _1));
    RETVAL = new PerlCommandHolder(aTHX_ id, func);
OUTPUT:
    RETVAL


INCLUDE: clients.xs
INCLUDE: helpers.xs
INCLUDE: messages.xs

