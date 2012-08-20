MODULE = Eir            PACKAGE = Eir::Client

string
Client::nick()

string
Client::user()

string
Client::host()

string
Client::nuh()

string
Client::account()

int
Client::has_privilege(...)
CODE:
    const char *channel = "", *priv;
    if (items > 2)
    {
        channel = SvPV_nolen(ST(1));
        priv = SvPV_nolen(ST(2));
    }
    else
    {
        priv = SvPV_nolen(ST(1));
    }
    RETVAL = THIS->privs().has_privilege(channel, priv);
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


