#include "settings.h"

#include <map>

#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>

using namespace eir;
using namespace paludis;

template class paludis::WrappedForwardIterator<GlobalSettingsManager::IteratorTag, std::pair<const std::string, Value> >;
template class paludis::InstantiationPolicy<GlobalSettingsManager, paludis::instantiation_method::SingletonTag>;

namespace paludis
{
    template <>
    struct Implementation<GlobalSettingsManager>
    {
        std::map<std::string, Value> _map;
    };
}

GlobalSettingsManager::iterator GlobalSettingsManager::begin()
{
    return _imp->_map.begin();
}

GlobalSettingsManager::iterator GlobalSettingsManager::end()
{
    return _imp->_map.end();
}

GlobalSettingsManager::iterator GlobalSettingsManager::find(std::string name)
{
    return _imp->_map.find(name);
}

Value& GlobalSettingsManager::get(std::string name)
{
    return _imp->_map[name];
}

Value GlobalSettingsManager::get_with_default(std::string name, Value _default)
{
    iterator it = find(name);
    if (it == end())
        return _default;
    return it->second;
}

bool GlobalSettingsManager::add(std::string name, Value v)
{
    return _imp->_map.insert(std::make_pair(name, v)).second;
}

size_t GlobalSettingsManager::remove(std::string n)
{
    return _imp->_map.erase(n);
}

void GlobalSettingsManager::remove(GlobalSettingsManager::iterator it)
{
    _imp->_map.erase(it.underlying_iterator<std::map<std::string, Value>::iterator>());
}

GlobalSettingsManager::GlobalSettingsManager()
    : PrivateImplementationPattern<GlobalSettingsManager>(new Implementation<GlobalSettingsManager>)
{
}

GlobalSettingsManager::~GlobalSettingsManager()
{
}
