#ifndef type_maps_hh
#define type_maps_hh

template <typename T>
struct PerlClassMap
{
};

#define TYPE_MAP_(t, n) \
    inline SV* sv_from(pTHX_ t *p) { \
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
TYPE_MAP(Channel)
TYPE_MAP(Membership)
TYPE_MAP(Filter)
TYPE_MAP(CommandRegistry)

TYPE_MAP_(const Message, "Eir::Message")

#ifndef TYPE_MAPS_LIGHT

TYPE_MAP_INTERNAL(BotClientHash)
TYPE_MAP_INTERNAL(BotChannelHash)
TYPE_MAP_INTERNAL(ClientMembershipHash)
TYPE_MAP_INTERNAL(ChannelMembershipHash)
TYPE_MAP_INTERNAL(PerlHolder)

TYPE_MAP_INTERNAL(ArrayValueWrapper)
TYPE_MAP_INTERNAL(HashValueWrapper)

#endif

#endif
