
MODULE = Eir            PACKAGE = Eir::Settings

SV*
Add(const char *name, SV *val)
CODE:
    GlobalSettingsManager::get_instance()->add(name, value_from_sv(aTHX_ val));
    RETVAL = sv_from_value(aTHX_ &GlobalSettingsManager::get_instance()->get(name));
OUTPUT:
    RETVAL


SV*
Find(const char *name)
CODE:
    RETVAL = sv_from_value(aTHX_ &GlobalSettingsManager::get_instance()->get(name));
OUTPUT:
    RETVAL



MODULE = Eir            PACKAGE = Eir::Internal::ArrayValueWrapper

SV *
ArrayValueWrapper::FETCH(int idx)
CODE:
    RETVAL = THIS->FETCH(aTHX_ idx);
OUTPUT:
    RETVAL

int
ArrayValueWrapper::FETCHSIZE()
CODE:
    RETVAL = THIS->FETCHSIZE(aTHX);
OUTPUT:
    RETVAL

void
ArrayValueWrapper::STORE(int idx, SV *sv)
CODE:
    THIS->STORE(aTHX_ idx, sv);

void
ArrayValueWrapper::STORESIZE(int newsize)
CODE:
    THIS->STORESIZE(aTHX_ newsize);

void
ArrayValueWrapper::EXTEND(int by)
CODE:
    THIS->_EXTEND(aTHX_ by);

int
ArrayValueWrapper::EXISTS(int idx)
CODE:
    RETVAL = THIS->EXISTS(aTHX_ idx);
OUTPUT:
    RETVAL

void
ArrayValueWrapper::DELETE(int idx)
CODE:
    THIS->DELETE(aTHX_ idx);

void
ArrayValueWrapper::CLEAR()
CODE:
    THIS->CLEAR(aTHX);

void
ArrayValueWrapper::PUSH(...)
CODE:
    // Start at 1; assume ST(0) == THIS
    for (int i=1; i < items; ++i)
        THIS->PUSH(aTHX_ ST(i));

SV *
ArrayValueWrapper::POP()
CODE:
    RETVAL = THIS->POP(aTHX);
OUTPUT:
    RETVAL

SV *
ArrayValueWrapper::SHIFT()
CODE:
    RETVAL = THIS->SHIFT(aTHX);
OUTPUT:
    RETVAL

void
ArrayValueWrapper::UNSHIFT()
CODE:
    for (int i=1; i < items; ++i)
        THIS->UNSHIFT(aTHX_ ST(i));

void
ArrayValueWrapper::DESTROY()

MODULE = Eir            PACKAGE = Eir::Internal::HashValueWrapper

SV *
HashValueWrapper::FETCH(SV *key)
CODE:
    RETVAL = THIS->FETCH(aTHX_ key);
OUTPUT:
    RETVAL

void
HashValueWrapper::STORE(SV *key, SV *val)
CODE:
    THIS->STORE(aTHX_ key, val);

void
HashValueWrapper::DELETE(SV *key)
CODE:
    THIS->DELETE(aTHX_ key);

void
HashValueWrapper::CLEAR()
CODE:
    THIS->CLEAR(aTHX);

int
HashValueWrapper::EXISTS(SV *key)
CODE:
    RETVAL = THIS->EXISTS(aTHX_ key);
OUTPUT:
    RETVAL

SV *
HashValueWrapper::FIRSTKEY()
CODE:
    RETVAL = THIS->FIRSTKEY(aTHX);
OUTPUT:
    RETVAL

SV *
HashValueWrapper::NEXTKEY(SV *prevkey)
CODE:
    RETVAL = THIS->NEXTKEY(aTHX_ prevkey);
OUTPUT:
    RETVAL

void
HashValueWrapper::DESTROY()
