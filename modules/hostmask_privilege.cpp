#include "eir.h"
#include "handler.h"
#include "match.h"

#include <functional>
#include <iostream>

using namespace eir;
using namespace std::tr1::placeholders;

struct HostmaskPrivilege : CommandHandlerBase<HostmaskPrivilege>, Module
{
    typedef std::set<std::pair<std::string, std::string> > priv_list;
    priv_list priv_entries;

    void recalculate_privileges(const Message *m)
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

        std::vector<std::string>::const_iterator it = m->args.begin();
        ++it;
        for( ; it != m->args.end(); ++it)
        {
            priv_entries.insert(make_pair(m->args[0], *it));
            m->source.reply("Added privilege " + *it + " for " + m->args[0]);
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

        priv_list::iterator it = priv_entries.begin(), ite = priv_entries.end();
        for ( ; it != ite; ++it)
        {
            if (it->first == m->args[0] && match(m->args[1], it->second))
            {
                m->source.reply("Removing privilege " + it->second + " from " + it->first);
                priv_entries.erase(it++);
            }
        }
    }

    void set_privileges(Client::ptr c)
    {
        priv_list::iterator it = priv_entries.begin(), ite = priv_entries.end();

        for ( ; it != ite; ++it)
        {
            std::string s1 = it->first, s2 = it->second;
            if (match(it->first, c->nuh()))
                c->privs().add_privilege(it->second);
        }
    }

    void set_client_privileges(const Message *m)
    {
        if (m->source.client)
            set_privileges(m->source.client);
    }

    void clear_host_privileges(const Message *m)
    {
        priv_entries.clear();

        for (Bot::ClientIterator it = m->bot->begin_clients(), ite = m->bot->end_clients(); it != ite; ++it)
            (*it)->privs().clear();
    }

    CommandHolder add_id, remove_id, client_id, clear_id;

    HostmaskPrivilege()
    {
        client_id = add_handler(filter_command_type("new_client",sourceinfo::Internal),
                                &HostmaskPrivilege::set_client_privileges);
        add_id = add_handler(filter_command_privilege("host_privilege", "admin").or_config(),
                                &HostmaskPrivilege::add_privilege_entry);
        remove_id = add_handler(filter_command_privilege("del_host_privilege", "admin").or_config(),
                                &HostmaskPrivilege::remove_privilege_entry);
        clear_id = add_handler(filter_command_type("clear_lists", sourceinfo::ConfigFile),
                                &HostmaskPrivilege::clear_host_privileges);
    }
};

MODULE_CLASS(HostmaskPrivilege)
