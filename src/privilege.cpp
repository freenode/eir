#include "privilege.h"

#include <set>

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>

using namespace eir;

namespace paludis
{
    template <>
    struct Implementation<PrivilegeSet>
    {
        std::set<std::pair<std::string, std::string> > privs;
    };
}

PrivilegeSet::iterator PrivilegeSet::begin()
{
    return _imp->privs.begin();
}

PrivilegeSet::iterator PrivilegeSet::end()
{
    return _imp->privs.end();
}

bool PrivilegeSet::has_privilege(std::string c, std::string p)
{
    return _imp->privs.find(make_pair(c, p)) != _imp->privs.end() ||
           _imp->privs.find(make_pair("", p)) != _imp->privs.end();
}

void PrivilegeSet::add_privilege(std::string c, std::string p)
{
    _imp->privs.insert(make_pair(c, p));
}

bool PrivilegeSet::has_privilege(std::string p)
{
    return _imp->privs.find(make_pair("", p)) != _imp->privs.end();
}

void PrivilegeSet::add_privilege(std::string p)
{
    _imp->privs.insert(make_pair("", p));
}

void PrivilegeSet::clear()
{
    _imp->privs.clear();
}

PrivilegeSet::PrivilegeSet()
    : paludis::PrivateImplementationPattern<PrivilegeSet>(new paludis::Implementation<PrivilegeSet>())
{
}

PrivilegeSet::~PrivilegeSet()
{
}

template class paludis::WrappedForwardIterator<eir::PrivilegeSet::PrivilegeIteratorTag,
                                               const std::pair<std::string, std::string> >;

#include "handler.h"
#include "bot.h"
#include "settings.h"

namespace
{
    struct ClearPrivs : public CommandHandlerBase<ClearPrivs>
    {
        void do_clear(const Message *m)
        {
            for (Bot::ClientIterator it = m->bot->begin_clients(); it != m->bot->end_clients(); ++it)
                (*it)->privs().clear();
        }
        CommandHolder id;

        ClearPrivs()
            : id(add_handler(filter_command_type("recalculate_privileges", sourceinfo::Internal),
                            &ClearPrivs::do_clear, Message::first))
        {
        }
    };

    ClearPrivs privilege_clearer;

    struct PrivilegeSaver : CommandHandlerBase<PrivilegeSaver>
    {
        void save_event()
        {
            save(0);
        }

        void save(const Message *)
        {
            StorageManager::get_instance()->Save(
                GlobalSettingsManager::get_instance()->get("privileges"), "privileges");
        }

        void load(const Message *)
        {
            GlobalSettingsManager::get_instance()->get("privileges") =
                StorageManager::get_instance()->Load("privileges");
        }

        EventHolder save_evt;
        CommandHolder config_loaded_id, on_shutdown_id;

        PrivilegeSaver()
            : save_evt(add_recurring_event(300, &PrivilegeSaver::save_event)),
              config_loaded_id(add_handler(filter_command_type("config_loaded", sourceinfo::Internal),
                          &PrivilegeSaver::load)),
              on_shutdown_id(add_handler(filter_command_type("shutting_down", sourceinfo::Internal),
                          &PrivilegeSaver::save))
        {
        }
    };

    PrivilegeSaver privilege_saver;
}
