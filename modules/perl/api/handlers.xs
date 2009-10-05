
MODULE = Eir            PACKAGE = Eir::CommandRegistry

PerlHolder *
add_handler(Filter *filter, SV *func, ...)
CODE:
    bool quiet = false;
    Message::Order order = Message::normal;
    if (items > 2)
        quiet = (bool)SvIV(ST(2));
    if (items > 3)
        order = (Message::Order)SvIV(ST(3));
    CommandRegistry::id id = CommandRegistry::get_instance()->add_handler(
                                *filter,
                                std::bind(call_perl<PerlContext::Void, const char *, SV*, const Message *>,
                                            aTHX_ "Eir::Init::call_wrapper", func, _1),
                                quiet,
                                order);
    RETVAL = new PerlCommandHolder(aTHX_ id, func);
OUTPUT:
    RETVAL

MODULE = Eir            PACKAGE = Eir::EventManager

PerlHolder *
add_event(int time, SV *func)
CODE:
    EventManager::id id = EventManager::get_instance()->add_event(
                                time,
                                std::bind(call_perl<PerlContext::Void, const char *, SV*>,
                                            aTHX_ "Eir::Init::call_wrapper", func));
    RETVAL = new PerlEventHolder(aTHX_ id, func);
OUTPUT:
    RETVAL

PerlHolder *
add_recurring_event(int time, SV *func)
CODE:
    EventManager::id id = EventManager::get_instance()->add_recurring_event(
                                time,
                                std::bind(call_perl<PerlContext::Void, const char *, SV*>,
                                            aTHX_ "Eir::Init::call_wrapper", func));
    RETVAL = new PerlEventHolder(aTHX_ id, func);
OUTPUT:
    RETVAL
