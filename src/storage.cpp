#include "storage.h"
#include "handler.h"

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>

#include <list>
#include <set>

using namespace eir;
using namespace paludis;

template class paludis::InstantiationPolicy<StorageManager, instantiation_method::SingletonTag>;

namespace
{
    struct BackendData
    {
        StorageManager::BackendId id;
        std::string type;
        StorageBackend *be;
    };

    typedef std::list<BackendData> BackendList;
}

namespace paludis
{
    template <>
    struct Implementation<StorageManager>
    {
        BackendList backends;

        BackendList::iterator find_by_id(StorageManager::BackendId id)
        {
            BackendList::iterator it;
            for (it = backends.begin(); it != backends.end(); ++it )
            {
                if (it->id == id)
                    break;
            }
            return it;
        }

        BackendList::iterator find_by_type(std::string type)
        {
            BackendList::iterator it;
            for (it = backends.begin(); it != backends.end(); ++it )
            {
                if (it->type == type)
                    break;
            }
            return it;
        }

        void split_storage_dest(const std::string & dest, std::string & type, std::string & target)
        {
            std::string::size_type colon = dest.find(':');

            if (colon == std::string::npos)
            {
                if (!default_backend)
                    throw StorageError("No storage backend specified and no default set");

                type = default_backend->type;
                target = dest;
            }
            else
            {
                type = dest.substr(0, colon);
                target = dest.substr(colon + 1);
            }
        }

        BackendData *default_backend;

        std::set<std::pair<const Value *, std::string>> auto_saves;

        void do_auto_save(const Value *v, std::string dest)
        {
            auto_saves.insert(make_pair(v, dest));
        }

        void do_auto_saves(const Message *)
        {
            for (auto it = auto_saves.begin(); it != auto_saves.end(); ++it)
            {
                do_save(*it->first, it->second);
            }
        }

        void do_save(const eir::Value & v, std::string dest)
        {
            std::string type, destination;
            split_storage_dest(dest, type, destination);

            BackendList::iterator it = find_by_type(type);

            if (it == backends.end())
                throw StorageError("No such storage type '" + type + "' has been loaded");

            it->be->Save(v, destination);
        }

        EventHolder auto_save_event;
        CommandHolder shutdown_save_command;

        Implementation()
            : default_backend(0)
        {
            auto_save_event = EventManager::get_instance()->add_recurring_event(120,
                                std::bind(&Implementation<StorageManager>::do_auto_saves, this, (const Message *)0));
            shutdown_save_command = CommandRegistry::get_instance()->add_handler(
                                filter_command_type("shutting_down", sourceinfo::Internal),
                                std::bind(&Implementation<StorageManager>::do_auto_saves, this, std::placeholders::_1));
        }
    };
}

StorageManager::StorageManager()
    : PrivateImplementationPattern<StorageManager>(new Implementation<StorageManager>)
{
}

StorageManager::~StorageManager()
{
}

void StorageManager::unregister_backend(StorageManager::BackendId id)
{
    BackendList::iterator it = _imp->find_by_id(id);
    if (it != _imp->backends.end())
        _imp->backends.erase(it);
}

StorageManager::BackendId StorageManager::register_backend(std::string type, StorageBackend *be)
{
    static StorageManager::BackendId next_id = 1;

    if (_imp->find_by_type(type) != _imp->backends.end())
        throw InternalError("Attempted to register the same backend type (" + type + ") twice.");

    BackendData data;
    data.id = next_id++;
    data.type = type;
    data.be = be;

    _imp->backends.push_back(data);

    return data.id;
}

void StorageManager::auto_save(const eir::Value * v, std::string dest)
{
    _imp->do_auto_save(v, dest);
}

void StorageManager::Save(const eir::Value & v, std::string dest)
{
    _imp->do_save(v, dest);
}

eir::Value StorageManager::Load(std::string src)
{
    std::string type, source;
    _imp->split_storage_dest(src, type, source);

    BackendList::iterator it = _imp->find_by_type(type);

    if (it == _imp->backends.end())
        throw StorageError("No such storage type '" + type + "' has been loaded");

    return it->be->Load(source);
}

std::string StorageManager::default_backend()
{
    return _imp->default_backend->type;
}

void StorageManager::default_backend(std::string type)
{
    BackendList::iterator it = _imp->find_by_type(type);

    if (it == _imp->backends.end())
        throw StorageError("No such storage type '" + type + "' has been loaded");

    _imp->default_backend = &*it;
}

#include "handler.h"

namespace
{
    struct SetDefaultBackend : CommandHandlerBase<SetDefaultBackend>
    {
        void set(const Message *m)
        {
            if (m->args.empty())
                return;

            StorageManager::get_instance()->default_backend(m->args[0]);
        }

        CommandHolder id;
        SetDefaultBackend()
            : id(add_handler(filter_command_type("default_storage", sourceinfo::ConfigFile),
                        &SetDefaultBackend::set))
        {
        }
    };

    SetDefaultBackend default_setter;
}


