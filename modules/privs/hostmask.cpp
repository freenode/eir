#include "eir.h"
#include "handler.h"
#include "match.h"

using namespace eir;

struct HostmaskPrivilege : CommandHandlerBase<HostmaskPrivilege>, Module
{
    Value & priv_entries() { return GlobalSettingsManager::get_instance()->get("privileges"); }
    Value & priv_types() { return GlobalSettingsManager::get_instance()->get("privilege_types"); }

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
