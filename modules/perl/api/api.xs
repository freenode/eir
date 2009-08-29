#include "stupid_perl_workarounds.h"

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "eir.h"

MODULE = Eir            PACKAGE = Eir

Bot *
find_bot(name)
    char *name
CODE:
    RETVAL = eir::BotManager::get_instance()->find(name);
