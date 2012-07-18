#include "eir.h"
#include "handler.h"
#include "match.h"

using namespace eir;

struct HostmaskPrivilege : CommandHandlerBase<HostmaskPrivilege>, Module
{
    Value * _cache_priv_entries;
    Value * _cache_priv_types;
    Value & priv_entries() { if (!_cache_priv_entries) _cache_priv_entries = &GlobalSettingsManager::get_instance()->get("privileges"); return *_cache_priv_entries; }
    Value & priv_types() { if (!_cache_priv_types) _cache_priv_types = &GlobalSettingsManager::get_instance()->get("privilege_types"); return *_cache_priv_types; }

    void calculate_hostmask_privileges(const Message *m)
    {
        for (auto it = priv_entries().begin(); it != priv_entries().end(); ++it)
        {
            if ((*it)["type"] == "host" &&
                match((*it)["match"], m->source.client->nuh()))
            {
                m->source.client->privs().add_privilege((*it)["channel"], (*it)["priv"]);
            }
        }
    }

    CommandHolder calc_handler;

    HostmaskPrivilege()
        : _cache_priv_entries(0), _cache_priv_types(0)
    {
        calc_handler = add_handler(filter_command_type("calculate_client_privileges", sourceinfo::Internal),
                                    &HostmaskPrivilege::calculate_hostmask_privileges);

        priv_types()["host"] = 1;
    }

    ~HostmaskPrivilege()
    {
        priv_types()["host"] = 0;
    }
};

MODULE_CLASS(HostmaskPrivilege)
