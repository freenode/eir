#include "stupid_perl_workarounds.h"

#include <EXTERN.h>
#include <perl.h>

#include "eir.h"

#include "value_wrapper.hh"
#include "perl_helpers.hh"
#include "HashWrappers.h"

using namespace eir;
using namespace eir::perl;

#include "util/type_maps.h"

namespace eir { namespace perl {
    Value* value_star_from_sv(pTHX_ SV *sv)
    {
        if (SvMAGICAL(SvRV(sv)))
        {
            MAGIC *mg = mg_find(SvRV(sv), PERL_MAGIC_tied);
            if (!mg || !mg->mg_obj)
                return 0;

            SV *obj = mg->mg_obj;
            if (!sv_isobject(obj) || SvTYPE(SvRV(obj)) != SVt_PVMG)
                return 0;

            if (sv_derived_from(obj, PerlClassMap<ArrayValueWrapper*>::name()))
                return ((ArrayValueWrapper*)SvIV((SV*)SvRV(obj)))->_value;
            else if (sv_derived_from(obj, PerlClassMap<HashValueWrapper*>::name()))
                return ((HashValueWrapper*)SvIV((SV*)SvRV(obj)))->_value;
        }

        return 0;
    }

    Value value_from_sv(pTHX_ SV *sv)
    {
        Value *v = value_star_from_sv(aTHX_ sv);
        if (v)
            return *v;
        else if (SvIOK(sv) && SvIOKp(sv))
            return Value(SvIV(sv));
        else if (SvPOK(sv) && SvPOKp(sv))
            return Value(SvPV_nolen(sv));
        else if (SvROK(sv))
        {
            if (SvTYPE(SvRV(sv)) == SVt_PVAV)
            {
                Value ret(Value::array);
                AV *array = (AV*)SvRV(sv);
                int len = av_len(array) + 1;
                for (int i=0; i < len; ++i)
                    ret.push_back(value_from_sv(aTHX_ *av_fetch(array, i, 0)));
                return ret;
            }
            else if (SvTYPE(SvRV(sv)) == SVt_PVHV)
            {
                Value ret(Value::kvarray);
                HV *hash = (HV*)SvRV(sv);
                hv_iterinit(hash);
                SV *value;
                char *key;
                int keylen;
                while ((value = hv_iternextsv(hash, &key, &keylen)))
                {
                    ret[key] = value_from_sv(aTHX_ value);
                }
                return ret;
            }
        }
        return Value(Value::empty);
    }

    SV *sv_from_value(pTHX_ Value *v)
    {
        switch (v->Type())
        {
            case Value::empty:
                return &PL_sv_undef;
            case Value::integer:
                return newSViv(v->Int());
            case Value::string:
                return newSVpv(v->String().c_str(), 0);
            case Value::array:
                {
                    AV *array = newAV();
                    SV *tie = newSV(0);
                    sv_setref_pv(tie, PerlClassMap<ArrayValueWrapper*>::name(),
                                    new ArrayValueWrapper(aTHX_ v));
                    hv_magic(array, tie, PERL_MAGIC_tied);
                    return newRV_noinc((SV*)array);
                }
            case Value::kvarray:
                {
                    HV *hash = newHV();
                    SV *tie = newSV(0);
                    sv_setref_pv(tie, PerlClassMap<HashValueWrapper*>::name(),
                                    new HashValueWrapper(aTHX_ v));
                    hv_magic(hash, tie, PERL_MAGIC_tied);
                    return newRV_noinc((SV*)hash);
                }
        }
        return &PL_sv_undef;
    }
} }

ArrayValueWrapper::ArrayValueWrapper(pTHX_ Value *v)
    : _value(v)
{
    if (v->Type() != Value::array)
        Perl_croak(aTHX_ "Can't use an array wrapper for a value that's not an array");
}

SV *ArrayValueWrapper::FETCH(pTHX_ int idx)
{
    return sv_from_value(aTHX_ &_value->Array()[idx]);
}

void ArrayValueWrapper::STORE(pTHX_ int idx, SV *sv)
{
    _value->Array()[idx] = value_from_sv(aTHX_ sv);
}

void ArrayValueWrapper::STORESIZE(pTHX_ int newsize)
{
    int size = _value->Array().size();
    while (size < newsize)
    {
        _value->Array().push_back(Value());
        size++;
    }
    while (size > newsize)
    {
        _value->Array().erase(--size);
    }
}

void ArrayValueWrapper::_EXTEND(pTHX_ int)
{
    PERL_UNUSED_CONTEXT;
}

int ArrayValueWrapper::EXISTS(pTHX_ int idx)
{
    return (unsigned)idx < _value->Array().size() && _value->Array()[idx].Type() != Value::empty;
}

void ArrayValueWrapper::DELETE(pTHX_ int idx)
{
    _value->Array().erase(idx);
}

void ArrayValueWrapper::CLEAR(pTHX)
{
    *_value = Value(Value::array);
}

void ArrayValueWrapper::PUSH(pTHX_ SV *sv)
{
    _value->Array().push_back(value_from_sv(aTHX_ sv));
}

SV *ArrayValueWrapper::POP(pTHX)
{
    SV *ret = sv_from_value(aTHX_ &_value->Array().back());
    _value->Array().pop_back();
    return ret;
}

SV *ArrayValueWrapper::SHIFT(pTHX)
{
    SV *ret = sv_from_value(aTHX_ &_value->Array()[0]);
    _value->Array().erase(0);
    return ret;
}

void ArrayValueWrapper::UNSHIFT(pTHX_ SV *sv)
{
    _value->Array().insert(0, value_from_sv(aTHX_ sv));
}

HashValueWrapper::HashValueWrapper(pTHX_ Value *v)
    : _value(v)
{
    if (v->Type() != Value::kvarray)
        Perl_croak(aTHX_ "Can't use a hash wrapper for a value that isn't a k-v array");
}

SV* HashValueWrapper::FETCH(pTHX_ SV *key)
{
    return sv_from_value(&_value->KV()[SvPV_nolen(key)]);
}

void HashValueWrapper::STORE(pTHX_ SV *key, SV *value)
{
    _value->KV()[SvPV_nolen(key)] = value_from_sv(value);
}

void HashValueWrapper::DELETE(pTHX_ SV *key)
{
    _value->KV().erase(std::string(SvPV_nolen(key)));
}

void HashValueWrapper::CLEAR(pTHX)
{
    *_value = Value(Value::kvarray);
}

int HashValueWrapper::EXISTS(pTHX_ SV *key)
{
    return _value->KV().find(SvPV_nolen(key)) != _value->KV().end();
}

SV *HashValueWrapper::FIRSTKEY(pTHX)
{
    return _value->KV().empty() ? &PL_sv_undef : newSVpv(_value->KV().begin()->first.c_str(), 0);
}

SV *HashValueWrapper::NEXTKEY(pTHX_ SV *prevkey)
{
    auto prev = _value->KV().find(SvPV_nolen(prevkey));
    if (prev == _value->KV().end())
        return &PL_sv_undef;
    ++prev;
    if (prev == _value->KV().end())
        return &PL_sv_undef;
    return newSVpv(prev->first.c_str(), 0);
}

