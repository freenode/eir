#include "stupid_perl_workarounds.h"

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "eir.h"

#include "HashWrappers.h"
#include "perl_helpers.hh"
#include "value_wrapper.hh"

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


// Macros used to turn xsubpp's exception-handling stubs into something vaguely useful.
#define TRY try
#define BEGHANDLERS catch (eir::Exception & e) \
                        { snprintf(errbuf, sizeof errbuf, "%s (%s)", e.message().c_str(), e.what()); } \
                    catch (std::exception & e) \
                        { snprintf(errbuf, sizeof errbuf, "Unknown exception (%s)", e.what()); }
#define CATCHALL    catch (...) { const char *Xname = "Unknown error", *Xreason = "";
#define ENDHANDLERS }

MODULE = Eir            PACKAGE = Eir

Bot *
find_bot(const char *name)
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


INCLUDE: clients.xs
INCLUDE: helpers.xs
INCLUDE: messages.xs
INCLUDE: handlers.xs
INCLUDE: value.xs
INCLUDE: logger.xs
INCLUDE: storage.xs

