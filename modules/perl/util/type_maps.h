#ifndef type_maps_hh
#define type_maps_hh

template <typename T>
struct PerlClassMap
{
};

#define TYPE_MAP(x) \
    SV* sv_from(x *p) { \
        SV *ret = newSV(0); \
        sv_setref_pv(ret, "Eir::"#x, (void*)p); \
        return sv_2mortal(ret); \
    } \
    template <> \
    struct PerlClassMap<x*> { static const char *name() { return "Eir::"#x; } };

TYPE_MAP(Bot)
TYPE_MAP(Client)

#endif
