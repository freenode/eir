#ifndef type_maps_hh
#define type_maps_hh

template <typename T>
struct PerlClassMap
{
};

#define TYPE_MAP_(t, n) \
    SV* sv_from(t *p) { \
        SV *ret = newSV(0); \
        sv_setref_pv(ret, n, (void*)p); \
        return sv_2mortal(ret); \
    } \
    template <> \
    struct PerlClassMap<t*> { static const char *name() { return n; } };

#define TYPE_MAP(x) TYPE_MAP_(x, "Eir::"#x)
#define TYPE_MAP_INTERNAL(x) TYPE_MAP_(x,  "Eir::Internal::"#x)

TYPE_MAP(Bot)
TYPE_MAP(Client)

TYPE_MAP_INTERNAL(BotClientHash)

#endif
