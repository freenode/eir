#include "stupid_perl_workarounds.h"

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "eir.h"

using namespace eir;
using std::string;

#include "util/type_maps.h"

MODULE = Eir            PACKAGE = Eir

Bot *
find_bot(name)
    char *name
CODE:
    RETVAL = BotManager::get_instance()->find(name);

MODULE = Eir            PACKAGE = Eir::Bot

void
Bot::send(char* name)
