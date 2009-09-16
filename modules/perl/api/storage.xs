
MODULE = Eir            PACKAGE = Eir::Storage

void
Save(SV *ref, const char *dest)
CODE:
    Value *value = 0;

    if (!SvROK(ref))
        Perl_croak(aTHX_ "Eir::Storage::Save() -- argument isn't a reference");

    SV *tie = SvRV(ref);
    MAGIC *mg = mg_find(tie, PERL_MAGIC_tied);
    if (!mg || !mg->mg_obj)
        Perl_croak(aTHX_ "Eir::Storage::Save() -- argument isn't a tied Value reference");

    SV *obj = mg->mg_obj;

    if (sv_isobject(obj) && (SvTYPE(SvRV(obj)) == SVt_PVMG))
    {
        if (sv_derived_from(obj, PerlClassMap<ArrayValueWrapper*>::name()))
            value = ((ArrayValueWrapper*)SvIV((SV*)SvRV(obj)))->_value;
        else if (sv_derived_from(obj, PerlClassMap<HashValueWrapper*>::name()))
            value = ((HashValueWrapper*)SvIV((SV*)SvRV(obj)))->_value;
    }
    if (!value)
        Perl_croak(aTHX_ "Eir::Storage::Save() -- argument isn't a tied Value reference)");
    StorageManager::get_instance()->Save(*value, dest);
