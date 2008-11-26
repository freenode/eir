#include "modules.h"

#include <paludis/util/instantiation_policy-impl.hh>

#include <dlfcn.h>

using namespace eir;

template class paludis::InstantiationPolicy<ModuleRegistry, paludis::instantiation_method::SingletonTag>;

void ModuleRegistry::load(std::string name) throw(ModuleError)
{
    void *handle = dlopen(name.c_str(), RTLD_LOCAL|RTLD_NOW);

    if (!handle)
        throw ModuleError(dlerror());

    _modules.insert(make_pair(name, handle));
}

bool ModuleRegistry::unload(std::string name)
{
    std::map<std::string, id>::iterator it = _modules.find(name);
    if (it != _modules.end())
    {
        void *h = (*it).second;

        _modules.erase(it);

        if(dlclose(h) != 0)
            throw ModuleError(dlerror());

        return true;
    }
    return false;
}

bool ModuleRegistry::is_loaded(std::string name)
{
    return _modules.find(name) != _modules.end();
}
