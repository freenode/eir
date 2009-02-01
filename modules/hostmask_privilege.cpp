#include "eir.h"
#include "handler.h"
#include "match.h"

#include <functional>
#include <iostream>

using namespace eir;
using namespace std::tr1::placeholders;

struct HostmaskPrivilege : public CommandHandlerBase<HostmaskPrivilege>
{
    typedef std::list<std::pair<std::string, std::string> > priv_list;
    priv_list priv_entries;

    void add_privilege_entry(const Message *m)
    {
        if (m->source.type != sourceinfo::ConfigFile &&
                ( !m->source.client || !m->source.client->privs().has_privilege("admin")))
        {
            m->source.reply("Nope.");
            return;
        }

        if (m->args.size() < 2)
        {
            m->source.reply("Error: need two arguments for " + m->command);
            return;
        }

        std::vector<std::string>::const_iterator it = m->args.begin();
        ++it;
        for( ; it != m->args.end(); ++it)
        {
            priv_entries.push_back(make_pair(m->args[0], *it));
            m->source.reply("Added privilege " + *it + " for " + m->args[0]);
        }
    }

    void remove_privilege_entry(const Message *m)
    {
        if (m->source.type != sourceinfo::ConfigFile &&
                ( !m->source.client || !m->source.client->privs().has_privilege("admin")))
        {
            m->source.reply("Nope.");
            return;
        }

        if (m->args.size() < 2)
        {
            m->source.reply("Error: need two arguments for " + m->command);
            return;
        }

        priv_list::iterator it = priv_entries.begin(), ite = priv_entries.end();
        for ( ; it != ite; ++it)
        {
            if (it->first == m->args[0] && match(it->second, m->args[1]))
            {
                m->source.reply("Removing privilege " + it->second + " from " + it->first);
                priv_entries.erase(it++);
            }
        }
    }

    void set_client_privileges(const Message *m)
    {
        priv_list::iterator it = priv_entries.begin(), ite = priv_entries.end();

        for ( ; it != ite; ++it)
        {
            std::string s1 = it->first, s2 = it->second;
            if (match(it->first, m->source.client->nuh()))
                m->source.client->privs().add_privilege(it->second);
        }
    }

    CommandRegistry::id add_id, remove_id, client_id;

    HostmaskPrivilege()
    {
        client_id = add_handler("new_client", &HostmaskPrivilege::set_client_privileges);
        add_id = add_handler("host_privilege", &HostmaskPrivilege::add_privilege_entry);
        remove_id = add_handler("del_host_privilege", &HostmaskPrivilege::remove_privilege_entry);
    }

    ~HostmaskPrivilege()
    {
        remove_handler(client_id);
        remove_handler(add_id);
        remove_handler(remove_id);
    }
} host_priv;
