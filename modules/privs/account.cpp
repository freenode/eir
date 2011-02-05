#include "eir.h"
#include "handler.h"
#include "match.h"

using namespace eir;

struct AccountPrivilege : CommandHandlerBase<AccountPrivilege>, Module
{
    Value & priv_entries() { return GlobalSettingsManager::get_instance()->get("privileges")["global"]; }
    Value & priv_types() { return GlobalSettingsManager::get_instance()->get("privilege_types"); }

    void calculate_account_privileges(const Message *m)
    {
        for (auto it = priv_entries().begin(); it != priv_entries().end(); ++it)
        {
            if ((*it)["type"] == "account" &&
                (*it)["match"] == m->source.client->account())
            {
                m->source.client->privs().add_privilege((*it)["priv"]);
            }
        }
    }

    CommandHolder calc_handler;

    AccountPrivilege()
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
