#include "eir.h"
#include "handler.h"
#include "match.h"

using namespace eir;

struct PrivilegeHandler : CommandHandlerBase<PrivilegeHandler>, Module
{
    Value & priv_entries() { return GlobalSettingsManager::get_instance()->get("privileges")["global"]; }
    Value & priv_types() { return GlobalSettingsManager::get_instance()->get("privilege_types"); }

    Value make_priv_entry(std::string type, std::string match, std::string priv, bool config = false)
    {
        Value v(Value::kvarray);
        v["type"] = type;
        v["match"] = match;
        v["priv"] = priv;
        v["is_config"] = config;
        return v;
    }

    void add_privilege_entry(const Message *m)
    {
        if (m->args.size() < 3)
        {
            m->source.error("Need three arguments for " + m->command);
            return;
        }

        if (m->source.client)
            Logger::get_instance()->Log(m->bot, m->source.client, Logger::Command, m->source.raw);

        std::vector<std::string>::const_iterator it = m->args.begin();

        std::string type = *it++;
        std::string match = *it++;

        if (priv_types()[type] != 1)
        {
            m->source.error("Privilege type " + type + " not recognised");
            return;
        }

        for( ; it != m->args.end(); ++it)
        {
            priv_entries().push_back(make_priv_entry(type, match, *it, m->source.type == sourceinfo::ConfigFile));
            m->source.reply("Added privilege " + *it + " for " + match + " (" + type + ")");
            Logger::get_instance()->Log(m->bot, m->source.client, Logger::Admin,
                                        "Adding privilege " + *it + " for " + match + " (" + type + ")");
        }

        recalculate_privileges(m);
    }

    void remove_privilege_entry(const Message *m)
    {
        if (m->args.size() < 2)
        {
            m->source.error("Need two arguments for " + m->command);
            return;
        }

        for ( ValueArray::iterator it = priv_entries().begin(), ite = priv_entries().end(); it != ite; )
        {
            if ((*it)["match"] == m->args[0] && match(m->args[1], (*it)["priv"]))
            {
                if ((*it)["is_config"])
                {
                    m->source.error("Privilege " + (*it)["priv"] + " for " + (*it)["match"] +
                            " is defined in the configuration file and must be removed there.");
                    continue;
                }

                Logger::get_instance()->Log(m->bot, m->source.client, Logger::Admin,
                                            "Removing privilege " + (*it)["priv"] + " from " + (*it)["hostmask"]);
                m->source.reply("Removing privilege " + (*it)["priv"] + " from " + (*it)["hostmask"]);
                priv_entries().erase(it++);
            }
            else
                ++it;
        }

        recalculate_privileges(m);
    }

    void list_privs(const Message *m)
    {
        std::map<std::string, std::string> response, c_response;

        for (auto it = priv_entries().begin(); it != priv_entries().end(); ++it)
        {
            if ((*it)["is_config"])
                c_response[(*it)["match"]] += (*it)["priv"] + " ";
            else
                response[(*it)["match"]] += (*it)["priv"] + " ";
        }

        for (auto it = c_response.begin(); it != c_response.end(); ++it)
        {
            m->source.reply(" - " + it->first + " has privileges " + it->second + " (config)");
        }
        for (auto it = response.begin(); it != response.end(); ++it)
        {
            m->source.reply(" - " + it->first + " has privileges " + it->second);
        }
    }

    void set_client_privileges(Bot *b, Client::ptr c)
    {
        c->privs().clear();
        Message calc_client_privs(b, "calculate_client_privileges", sourceinfo::Internal, c);
        CommandRegistry::get_instance()->dispatch(&calc_client_privs);
    }

    void set_client_privileges_from(const Message *m)
    {
        set_client_privileges(m->bot, m->source.client);
    }

    void recalculate_privileges(const Message *m)
    {
        for (auto it = m->bot->begin_clients(); it != m->bot->end_clients(); ++it)
            set_client_privileges(m->bot, *it);
    }

    void clear_conf_privileges(const Message *)
    {
        for (auto it = priv_entries().begin(); it != priv_entries().end(); )
        {
            if ((*it)["is_config"])
                priv_entries().erase(it++);
            else
                ++it;
        }
    }

    CommandHolder add_id, add2_id, remove_id, client_id, recalc_id, clear_id, list_id;

    PrivilegeHandler()
    {
        client_id = add_handler(filter_command_type("new_client",sourceinfo::Internal),
                                &PrivilegeHandler::set_client_privileges_from);
        add_id = add_handler(filter_command_type("privilege", sourceinfo::ConfigFile),
                                &PrivilegeHandler::add_privilege_entry);
        add2_id = add_handler(filter_command_privilege("addpriv", "admin"),
                                &PrivilegeHandler::add_privilege_entry);
        remove_id = add_handler(filter_command_privilege("delpriv", "admin"),
                                &PrivilegeHandler::remove_privilege_entry);
        list_id = add_handler(filter_command_privilege("showprivs", "admin"),
                                &PrivilegeHandler::list_privs);
        recalc_id = add_handler(filter_command_type("recalculate_privileges", sourceinfo::Internal),
                                &PrivilegeHandler::recalculate_privileges);
        clear_id = add_handler(filter_command_type("clear_lists", sourceinfo::Internal),
                                &PrivilegeHandler::clear_conf_privileges);
    }
};

MODULE_CLASS(PrivilegeHandler)
