#ifndef value_wrapper_hh
#define value_wrapper_hh

namespace eir
{
    namespace perl
    {
        SV *sv_from_value(pTHX_ Value *, bool owned = false);
        Value value_from_sv(pTHX_ SV *);
        Value* value_star_from_sv(pTHX_ SV *);

        struct ArrayValueWrapper
        {
            Value *_value;
            bool _owned;

            ArrayValueWrapper(pTHX_ Value *, bool = false);
            ~ArrayValueWrapper();
            SV *FETCH(pTHX_ int);
            void STORE(pTHX_ int, SV *);
            void STORESIZE(pTHX_ int);
            void _EXTEND(pTHX_ int);
            int EXISTS(pTHX_ int);
            void DELETE(pTHX_ int);
            void CLEAR(pTHX);
            void PUSH(pTHX_ SV *);
            SV *POP(pTHX);
            SV *SHIFT(pTHX);
            void UNSHIFT(pTHX_ SV *);
        };

        struct HashValueWrapper
        {
            Value *_value;
            bool _owned;

            HashValueWrapper(pTHX_ Value *, bool = false);
            ~HashValueWrapper();
            SV *FETCH(pTHX_ SV*);
            void STORE(pTHX_ SV*, SV*);
            void DELETE(pTHX_ SV*);
            void CLEAR(pTHX);
            int EXISTS(pTHX_ SV*);
            SV* FIRSTKEY(pTHX);
            SV* NEXTKEY(pTHX_ SV*);
        };
    }
}

#endif
