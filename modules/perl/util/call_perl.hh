#ifndef call_perl_h
#define call_perl_h

#include <EXTERN.h>
#include <perl.h>

#include "exceptions.h"
#include <vector>

enum class PerlContext {
    Void,
    Scalar,
    Array
};

namespace eir
{
    struct PerlException : public eir::Exception
    {
        PerlException(std::string s) : Exception("Perl call raised error: " + s) { }
    };
}

namespace call_perl_internals
{
    typedef std::vector<SV*> sv_list;

    template <typename _First>
    void push_perl_args(pTHX_ sv_list& arglist, _First arg)
    {
        arglist.push_back(sv_from(aTHX_ arg));
    }

    template <typename _First, typename... _Rest>
    void push_perl_args(pTHX_ sv_list& arglist, _First firstarg, _Rest... rest)
    {
        arglist.push_back(sv_from(aTHX_ firstarg));
        push_perl_args(aTHX_ arglist, rest...);
    }

    int do_call_perl(pTHX_ const char *name, int flags)
    {
        return call_pv(name, flags);
    }

    int do_call_perl(pTHX_ SV *func, int flags)
    {
        return call_sv(func, flags);
    }

    template <PerlContext context>
    struct PerlCallAttrs
    {
    };

    template <>
    struct PerlCallAttrs<PerlContext::Void>
    {
        typedef void ReturnType;

        static ReturnType extract_return_value(sv_list&)
        {
        }
        static void cleanup_errors(pTHX)
        {
            PERL_UNUSED_CONTEXT;
        }

        enum { callflag = G_VOID | G_DISCARD };
    };
    template <>
    struct PerlCallAttrs<PerlContext::Scalar>
    {
        typedef sv_list::value_type ReturnType;

        static ReturnType extract_return_value(sv_list& v)
        {
            return v[0];
        }
        static void cleanup_errors(pTHX)
        {
            dSP;
            POPi;
        }

        enum { callflag = G_SCALAR };
    };
    template <>
    struct PerlCallAttrs<PerlContext::Array>
    {
        typedef sv_list ReturnType;

        static ReturnType extract_return_value(sv_list& v)
        {
            return v;
        }
        static void cleanup_errors(pTHX)
        {
            PERL_UNUSED_CONTEXT;
        }

        enum { callflag = G_ARRAY };
    };

    SV* sv_from(pTHX_ long i)
    {
        return sv_2mortal(newSViv(i));
    }
    SV* sv_from(pTHX_ double d)
    {
        return sv_2mortal(newSVnv(d));
    }
    SV* sv_from(pTHX_ const char *s)
    {
        return sv_2mortal(newSVpv(s, 0));
    }
    SV* sv_from(pTHX_ std::string s)
    {
        return sv_from(aTHX_ s.c_str());
    }
    SV* sv_from(pTHX_ SV *sv)
    {
        return sv;
    }
}

template <PerlContext _C, typename _Func, typename... ArgTypes>
typename call_perl_internals::PerlCallAttrs<_C>::ReturnType
call_perl(pTHX_ _Func func, ArgTypes... args)
{
    call_perl_internals::sv_list arglist;
    call_perl_internals::push_perl_args(aTHX_ arglist, args...);
    bool _Eval = true;

    int flags = call_perl_internals::PerlCallAttrs<_C>::callflag;
    if (_Eval)
        flags |= G_EVAL;

    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);

    for (auto it = arglist.begin(); it != arglist.end(); ++it)
        XPUSHs(*it);

    PUTBACK;
    int count = call_perl_internals::do_call_perl(aTHX_ func, flags);
    SPAGAIN;

    if (_Eval && SvTRUE(ERRSV))
    {
        call_perl_internals::PerlCallAttrs<_C>::cleanup_errors(aTHX);
        throw eir::PerlException(SvPV_nolen(ERRSV));
    }

    call_perl_internals::sv_list returnlist;
    for (int i=0; i < count; ++i)
    {
        returnlist.push_back(POPs);
    }

    PUTBACK;
    FREETMPS;
    LEAVE;

    return call_perl_internals::PerlCallAttrs<_C>::extract_return_value(returnlist);
}

#endif
