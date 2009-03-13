#include "modules.h"

#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

#include <list>

#include <dlfcn.h>

using namespace eir;
using namespace paludis;

template class paludis::InstantiationPolicy<ModuleRegistry, paludis::instantiation_method::SingletonTag>;

namespace
{
        struct loaded_module
        {
            std::string name;
            void *handle;
            Module *obj;
        };
}

namespace paludis
{
    template <>
    struct Implementation<ModuleRegistry>
    {
        std::list<loaded_module> modules;
    };
}

void ModuleRegistry::load(std::string name) throw(ModuleError)
{
    if (is_loaded(name))
        return;

    loaded_module mod;

    mod.name = name;
    mod.handle = dlopen(name.c_str(), RTLD_LOCAL|RTLD_NOW);
    mod.obj = 0;

    if (!mod.handle)
        throw ModuleError(dlerror());

    try
    {
        Module* (*create)() = reinterpret_cast<Module*(*)()>(
                                reinterpret_cast<uintptr_t>(dlsym(mod.handle, "create")));
        if (!create)
            throw ModuleError("Module " + name + " does not contain a create() function.");

        mod.obj = create();
        if (!mod.obj)
            throw ModuleError("Module initialisation failed in " + name);

        _imp->modules.push_back(mod);
    }
    catch (ModuleError &)
    {
        dlclose(mod.handle);

        throw;
    }
    catch (Exception & e)
    {
        dlclose(mod.handle);

        throw ModuleError(e.message());
    }
    catch (std::exception & e)
    {
        dlclose(mod.handle);

        throw ModuleError(e.what());
    }
}

bool ModuleRegistry::unload(std::string name)
{
    std::list<loaded_module>::iterator mod = _imp->modules.end();
    for (std::list<loaded_module>::iterator it = _imp->modules.begin(); it != _imp->modules.end(); ++it)
    {
        if (it->name == name)
            mod = it;
    }

    if (mod == _imp->modules.end())
        return false;

    if (mod->obj)
        delete mod->obj;
    mod->obj = 0;

    if(dlclose(mod->handle) != 0)
        throw ModuleError(dlerror());

    _imp->modules.erase(mod);

    return true;
}

bool ModuleRegistry::is_loaded(std::string name)
{
    for (std::list<loaded_module>::iterator it = _imp->modules.begin(); it != _imp->modules.end(); ++it)
    {
        if (it->name == name)
            return true;
    }
    return false;
}

ModuleRegistry::ModuleRegistry()
    : PrivateImplementationPattern<ModuleRegistry>(new Implementation<ModuleRegistry>)
{
}

ModuleRegistry::~ModuleRegistry()
{
}

Module::~Module()
{
}
