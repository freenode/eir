#include "eir.h"
#include "handler.h"
#include "match.h"

#include <algorithm>

using namespace eir;

struct PrivilegeHandler : CommandHandlerBase<PrivilegeHandler>, Module
{
    Value & priv_entries() { return GlobalSettingsManager::get_instance()->get("privileges"); }
    Value & priv_types() { return GlobalSettingsManager::get_instance()->get("privilege_types"); }

    Value make_priv_entry(std::string type, std::string match, std::string channel, std::string priv, bool config = false)
    {
        Value v(Value::kvarray);
        v["type"] = type;
        v["match"] = match;
        v["channel"] = channel;
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

        std::vector<std::string>::const_iterator it = m->args.begin();

        std::string type = *it++;
        std::string match = *it++;
        std::string channel;
        if ((*it)[0] == '#')
            channel = *it++;

        // Now that we've decided what arguments are what, we can check channel-specific privileges
        if (m->source.type != sourceinfo::ConfigFile
            && (!m->source.client || !m->source.client->privs().has_privilege(channel, "admin")))
        {
            return;
        }

        if (priv_types()[type] != 1)
        {
            m->source.error("Privilege type " + type + " not recognised");
            return;
        }

        for( ; it != m->args.end(); ++it)
        {
            priv_entries().push_back(make_priv_entry(type, match, channel, *it, m->source.type == sourceinfo::ConfigFile));
            std::string reply = "Added privilege " + *it;
            if (!channel.empty())
                reply += " in " + channel;
            reply += " for " + match + " (" + type + ")";
            m->source.reply(reply);
            Logger::get_instance()->Log(m->bot, m->source.client, Logger::Admin, reply);
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

        std::string match = m->args[0];
        std::string channel, privs;
        if (m->args[1][0] == '#')
        {
            channel = m->args[1];
            if (m->args.size() < 3)
                return;
            privs = m->args[2];
        }
        else
        {
            privs = m->args[1];
        }

        // Now that we've decided what arguments are what, we can check channel-specific privileges
        if (!m->source.client || !m->source.client->privs().has_privilege(channel, "admin"))
        {
            m->source.error("goat");
            return;
        }

        for ( ValueArray::iterator it = priv_entries().begin(); it != priv_entries().end(); )
        {
            if ((*it)["match"] == match
                && ::match(privs, (*it)["priv"])
                && (channel.empty() || channel == (*it)["channel"]))
            {
                if ((*it)["is_config"])
                {
                    m->source.error("Privilege " + (*it)["priv"] + " for " + (*it)["match"] +
                            " is defined in the configuration file and must be removed there.");
                    ++it;
                    continue;
                }

                Logger::get_instance()->Log(m->bot, m->source.client, Logger::Admin,
                                            "Removing privilege " + (*it)["priv"] + " from " + (*it)["match"]);
                m->source.reply("Removing privilege " + (*it)["priv"] + " from " + (*it)["match"]);
                it = priv_entries().erase(it);
            }
            else
                ++it;
        }

        recalculate_privileges(m);
    }

    void list_privs(const Message *m)
    {
        std::string channel;
        if (m->args.size() > 0 && m->args[0][0] == '#')
            channel = m->args[0];

        if (!m->source.client || !m->source.client->privs().has_privilege(channel, "admin"))
            return;

        std::map<std::string, std::string> response, c_response;

        for (auto it = priv_entries().begin(); it != priv_entries().end(); ++it)
        {
            if (!channel.empty() && (*it)["channel"] != channel)
                continue;

            std::map<std::string, std::string> & r = (*it)["is_config"] ? c_response : response;

            std::string privfmt = (*it)["priv"];
            if ((*it)["channel"])
                privfmt += "(" + (*it)["channel"] + ")";

            r[(*it)["match"]] += privfmt + " ";
        }

        for (auto it = c_response.begin(); it != c_response.end(); ++it)
        {
            m->source.reply(" - " + it->first + " has privileges " + it->second + "(config)");
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
        Value new_privs(Value::array);
        std::copy_if(priv_entries().Array().begin(), priv_entries().Array().end(), std::back_inserter(new_privs.Array()),
                [](Value & v) -> bool { return !(v["is_config"]); });
        priv_entries() = new_privs;
    }

    CommandHolder add_id, add2_id, remove_id, client_id, recalc_id, clear_id, list_id;

    PrivilegeHandler()
    {
        client_id = add_handler(filter_command_type("new_client",sourceinfo::Internal),
                                &PrivilegeHandler::set_client_privileges_from);
        add_id = add_handler(filter_command_type("privilege", sourceinfo::ConfigFile),
                                &PrivilegeHandler::add_privilege_entry);
        recalc_id = add_handler(filter_command_type("recalculate_privileges", sourceinfo::Internal),
                                &PrivilegeHandler::recalculate_privileges);
        clear_id = add_handler(filter_command_type("clear_lists", sourceinfo::Internal),
                                &PrivilegeHandler::clear_conf_privileges);
        // These do their own privilege checking. The lack of privilege filter is intentional.
        add2_id = add_handler(filter_command("addpriv"),
                                &PrivilegeHandler::add_privilege_entry);
        remove_id = add_handler(filter_command("delpriv"),
                                &PrivilegeHandler::remove_privilege_entry);
        list_id = add_handler(filter_command("showprivs"),
                                &PrivilegeHandler::list_privs);

        // Load stored privileges. Maintain upgrade path from previous versions.
        try
        {
            Value loaded_privs = StorageManager::get_instance()->Load("privileges");
            if (loaded_privs.Type() == Value::kvarray)
                priv_entries() = loaded_privs["global"];
            else
                priv_entries() = loaded_privs;
        }
        catch (std::exception &)
        {
            priv_entries() = Value(Value::array);
        }

        // Strip out old config file entries.
        clear_conf_privileges(0);

        // And set privs to auto-save
        StorageManager::get_instance()->auto_save(&priv_entries(), "privileges");
    }
};

MODULE_CLASS(PrivilegeHandler)
