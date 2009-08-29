#include "stupid_perl_workarounds.h"

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "eir.h"

using namespace eir;

MODULE = Eir            PACKAGE = Eir

SV *
find_bot(name)
    char *name
PPCODE:
    Bot *ret = eir::BotManager::get_instance()->find(name);
    if (!ret)
        XSRETURN_UNDEF;
    SV *realret = sv_newmortal();
    sv_setref_pv(realret, "Eir::Bot", (void*)ret);
    XPUSHs(realret);

MODULE = Eir            PACKAGE = Eir::Bot

void
Bot::send(char* name)
