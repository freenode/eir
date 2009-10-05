
MODULE = Eir            PACKAGE = Eir::Logger

int
Debug()
CODE:
    RETVAL = Logger::Debug;
OUTPUT:
    RETVAL

int
Command()
CODE:
    RETVAL = Logger::Command;
OUTPUT:
    RETVAL

int
Info()
CODE:
    RETVAL = Logger::Info;
OUTPUT:
    RETVAL

int
Privs()
CODE:
    RETVAL = Logger::Privs;
OUTPUT:
    RETVAL

int
Warning()
CODE:
    RETVAL = Logger::Warning;
OUTPUT:
    RETVAL

int
Raw()
CODE:
    RETVAL = Logger::Raw;
OUTPUT:
    RETVAL

int
Admin()
CODE:
    RETVAL = Logger::Admin;
OUTPUT:
    RETVAL

void
Log(...)
PPCODE:
    if (items < 2)
        Perl_croak(aTHX_ "usage: Eir::Logger::Log([bot,[client,]] type, text)");

    Bot *bot = 0;
    Client *client = 0;
    int type = 0;
    const char *text = 0;
    int argc = 0;
    if (sv_isobject(ST(argc)) && sv_derived_from(ST(argc), PerlClassMap<Bot*>::name()))
    {
        bot = (Bot*)SvIV((SV*)SvRV(ST(argc)));
        ++argc;
        if (sv_isobject(ST(argc)) && sv_derived_from(ST(argc), PerlClassMap<Client*>::name()))
        {
            client = (Client*)SvIV((SV*)SvRV(ST(argc)));
            ++argc;
        }
    }
    if (items < argc + 2)
        Perl_croak(aTHX_ "usage: Eir::Logger::Log([bot,[client,]] type, text)");

    type = SvIV(ST(argc));
    ++argc;
    text = SvPV_nolen(ST(argc));
    Logger::get_instance()->Log(bot, client, type, text);
