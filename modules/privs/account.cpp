#include "eir.h"
#include "handler.h"
#include "match.h"

using namespace eir;

struct AccountPrivilege : CommandHandlerBase<AccountPrivilege>, Module
{
    Value * _cache_priv_entries;
    Value * _cache_priv_types;
    Value & priv_entries() { if (!_cache_priv_entries) _cache_priv_entries = &GlobalSettingsManager::get_instance()->get("privileges"); return *_cache_priv_entries; }
    Value & priv_types() { if (!_cache_priv_types) _cache_priv_types = &GlobalSettingsManager::get_instance()->get("privilege_types"); return *_cache_priv_types; }

    void calculate_account_privileges(const Message *m)
    {
        for (auto it = priv_entries().begin(); it != priv_entries().end(); ++it)
        {
            if ((*it)["type"] == "account" &&
                (*it)["match"] == m->source.client->account())
            {
                m->source.client->privs().add_privilege((*it)["channel"], (*it)["priv"]);
            }
        }
    }

    CommandHolder calc_handler;

    AccountPrivilege()
        : _cache_priv_entries(0), _cache_priv_types(0)
    {
        calc_handler = add_handler(filter_command_type("calculate_client_privileges", sourceinfo::Internal),
                                    &AccountPrivilege::calculate_account_privileges);

        priv_types()["account"] = 1;
    }

    ~AccountPrivilege()
    {
        priv_types()["account"] = 0;
    }
};

MODULE_CLASS(AccountPrivilege)
