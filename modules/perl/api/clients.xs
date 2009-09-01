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


