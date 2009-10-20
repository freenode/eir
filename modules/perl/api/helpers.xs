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
    Perl_croak(aTHX_ "Tried to modify a read-only tied hash Bot::Clients");

void
BotClientHash::DELETE(char *nick)
CODE:
    Perl_croak(aTHX_ "Tried to modify a read-only tied hash Bot::Clients");

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
    Perl_croak(aTHX_ "Tried to modify a read-only tied hash Bot::Channels");

void
BotChannelHash::DELETE(char *nick)
CODE:
    Perl_croak(aTHX_ "Tried to modify a read-only tied hash Bot::Channels");

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


MODULE = Eir            PACKAGE = Eir::Internal::BotSettingsHash

BotSettingsHash *
TIEHASH(char *classname, Bot* bot)
CODE:
    RETVAL = new BotSettingsHash(bot);
OUTPUT:
    RETVAL

SV *
BotSettingsHash::FETCH(char *name)
CODE:
    RETVAL = THIS->FETCH(aTHX_ name);
OUTPUT:
    RETVAL

void
BotSettingsHash::STORE(char *name, SV *value)
CODE:
    THIS->STORE(aTHX_ name, value);

void
BotSettingsHash::DELETE(char *name)

int
BotSettingsHash::EXISTS(char *name)

string
BotSettingsHash::FIRSTKEY()

string
BotSettingsHash::NEXTKEY(char *prev)

void
BotSettingsHash::DESTROY()

int
BotSettingsHash::SCALAR()


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
    Perl_croak(aTHX_ "Tried to modify a read-only tied hash Bot::Channels");

void
ClientMembershipHash::DELETE(char *name)
CODE:
    Perl_croak(aTHX_ "Tried to modify a read-only tied hash Bot::Channels");

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
    Perl_croak(aTHX_ "Tried to modify a read-only tied hash Bot::Channels");

void
ChannelMembershipHash::DELETE(char *name)
CODE:
    Perl_croak(aTHX_ "Tried to modify a read-only tied hash Bot::Channels");

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


MODULE = Eir            PACKAGE = Eir::Internal::PerlHolder

void
PerlHolder::DESTROY()


