
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
