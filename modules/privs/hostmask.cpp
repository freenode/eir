#include "eir.h"
#include "handler.h"
#include "match.h"

using namespace eir;
using namespace std::tr1::placeholders;

struct HostmaskPrivilege : CommandHandlerBase<HostmaskPrivilege>, Module
{
    Value & priv_entries;
    Value conf_priv_entries;

    Value make_priv_entry(std::string hostmask, std::string priv)
    {
        Value v(Value::kvarray);
        v["hostmask"] = hostmask;
        v["priv"] = priv;
        return v;
    }

    void recalculate_host_privileges(const Message *m)
    {
        for (Bot::ClientIterator it = m->bot->begin_clients(), ite = m->bot->end_clients(); it != ite; ++it)
            set_privileges(*it);
    }

    void add_privilege_entry(const Message *m)
    {
        if (m->args.size() < 2)
        {
            m->source.error("Need two arguments for " + m->command);
            return;
        }

        Value& list = (m->source.type == sourceinfo::ConfigFile ? conf_priv_entries : priv_entries);

        if (m->source.client)
            Logger::get_instance()->Log(m->bot, m->source.client, Logger::Command, m->source.raw);

        std::vector<std::string>::const_iterator it = m->args.begin();
        ++it;
        for( ; it != m->args.end(); ++it)
        {
            list.push_back(make_priv_entry(m->args[0], *it));
            m->source.reply("Added privilege " + *it + " for " + m->args[0]);
            Logger::get_instance()->Log(m->bot, m->source.client, Logger::Admin,
                                        "Adding privilege " + *it + " for " + m->args[0]);
        }

        recalculate_host_privileges(m);
    }

    void remove_privilege_entry(const Message *m)
    {
        if (m->args.size() < 2)
        {
            m->source.error("Need two arguments for " + m->command);
            return;
        }

        for ( ValueArray::iterator it = priv_entries.begin(), ite = priv_entries.end(); it != ite; )
        {
            if ((*it)["hostmask"] == m->args[0] && match(m->args[1], (*it)["priv"]))
            {
                Logger::get_instance()->Log(m->bot, m->source.client, Logger::Admin,
                                            "Removing privilege " + (*it)["priv"] + " from " + (*it)["hostmask"]);
                m->source.reply("Removing privilege " + (*it)["priv"] + " from " + (*it)["hostmask"]);
                priv_entries.erase(it++);
            }
            else
                ++it;
        }

        for ( ValueArray::iterator it = conf_priv_entries.begin(), ite = conf_priv_entries.end(); it != ite; ++it)
        {
            if ((*it)["hostmask"] == m->args[0] && match(m->args[1], (*it)["priv"]))
            {
                m->source.error("Privilege " + (*it)["priv"] + " for " + (*it)["hostmask"] +
                        " is defined in the config file and cannot be removed");
            }
        }

        dispatch_internal_message(m->bot, "recalculate_privileges");
    }

    void set_privileges(Client::ptr c)
    {
        for ( ValueArray::iterator it = priv_entries.begin(), ite = priv_entries.end(); it != ite; ++it)
            if (match((*it)["hostmask"], c->nuh()))
                c->privs().add_privilege((*it)["priv"]);

        for ( ValueArray::iterator it = conf_priv_entries.begin(), ite = conf_priv_entries.end(); it != ite; ++it)
            if (match((*it)["hostmask"], c->nuh()))
                c->privs().add_privilege((*it)["priv"]);
    }

    void set_client_privileges(const Message *m)
    {
        if (m->source.client)
            set_privileges(m->source.client);
    }

    void clear_conf_privileges(const Message *)
    {
        conf_priv_entries.clear();
    }

    CommandHolder add_id, add2_id, remove_id, client_id, recalc_id, clear_id;

    HostmaskPrivilege()
        : priv_entries(GlobalSettingsManager::get_instance()->get("privileges")["global"]["hostmask"])
    {
        client_id = add_handler(filter_command_type("new_client",sourceinfo::Internal),
                                &HostmaskPrivilege::set_client_privileges);
        add_id = add_handler(filter_command_type("host_privilege", sourceinfo::ConfigFile),
                                &HostmaskPrivilege::add_privilege_entry);
        add2_id = add_handler(filter_command_privilege("addpriv", "admin"),
                                &HostmaskPrivilege::add_privilege_entry);
        remove_id = add_handler(filter_command_privilege("delpriv", "admin"),
                                &HostmaskPrivilege::remove_privilege_entry);
        recalc_id = add_handler(filter_command_type("recalculate_privileges", sourceinfo::Internal),
                                &HostmaskPrivilege::recalculate_host_privileges);
        clear_id = add_handler(filter_command_type("clear_lists", sourceinfo::Internal),
                                &HostmaskPrivilege::clear_conf_privileges);
    }
};

MODULE_CLASS(HostmaskPrivilege)
