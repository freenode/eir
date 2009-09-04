
MODULE = Eir            PACKAGE = Eir::Settings

void
Add(const char *name, SV *val)
CODE:
    if (!GlobalSettingsManager::get_instance()->add(name, value_from_sv(val)))
        Perl_croak(aTHX_ "Failed to add setting");


SV*
Find(const char *name)
CODE:
    RETVAL = sv_from_value(&GlobalSettingsManager::get_instance()->get(name));
OUTPUT:
    RETVAL



MODULE = Eir            PACKAGE = Eir::Internal::ArrayValueWrapper

SV *
ArrayValueWrapper::FETCH(int idx)

void
ArrayValueWrapper::STORE(int idx, SV *sv)

void
ArrayValueWrapper::STORESIZE(int newsize)

void
ArrayValueWrapper::EXTEND(int by)
CODE:
    THIS->_EXTEND(by);

int
ArrayValueWrapper::EXISTS(int idx)

void
ArrayValueWrapper::DELETE(int idx)

void
ArrayValueWrapper::CLEAR()

void
ArrayValueWrapper::PUSH()
CODE:
    // Start at 1; assume ST(0) == THIS
    for (int i=1; i < items; ++i)
        THIS->PUSH(ST(i));

SV *
ArrayValueWrapper::POP()

SV *
ArrayValueWrapper::SHIFT()

void
ArrayValueWrapper::UNSHIFT()
CODE:
    for (int i=1; i < items; ++i)
        THIS->UNSHIFT(ST(i));


MODULE = Eir            PACKAGE = Eir::Internal::HashValueWrapper

SV *
HashValueWrapper::FETCH(SV *key)

void
HashValueWrapper::STORE(SV *key, SV *val)

void
HashValueWrapper::DELETE(SV *key)

void
HashValueWrapper::CLEAR()

int
HashValueWrapper::EXISTS(SV *key)

SV *
HashValueWrapper::FIRSTKEY()

SV *
HashValueWrapper::NEXTKEY(SV *prevkey)
