
MODULE = Eir            PACKAGE = Eir::Storage

void
Save(SV *ref, const char *dest)
CODE:
    StorageManager::get_instance()->Save(value_from_sv(aTHX_ ref), dest);

void
auto_save(SV *ref, const char *dest)
CODE:
    Value *v = value_star_from_sv(aTHX_ ref);
    if (!v)
        Perl_croak(aTHX_ "Eir::Storage::auto_save requires a persistent Value reference");

    StorageManager::get_instance()->auto_save(v, dest);

SV *
Load(const char *source)
CODE:
    Value *v = new Value;
    *v = StorageManager::get_instance()->Load(source);
    RETVAL = sv_from_value(aTHX_ v, true);
OUTPUT:
    RETVAL
