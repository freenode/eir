#ifndef modules_h
#define modules_h

#include <string>
#include "exceptions.h"
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace eir {

    class ModuleRegistry : public paludis::InstantiationPolicy<ModuleRegistry, paludis::instantiation_method::SingletonTag>,
                           public paludis::PrivateImplementationPattern<ModuleRegistry>
    {
        public:
            void load(std::string) throw(ModuleError);
            bool unload(std::string);

            bool is_loaded(std::string);

            ModuleRegistry();
            ~ModuleRegistry();
    };

    // A module's create() function should return a pointer to one of these. It will be destroyed
    // before unloading the module.
    class Module
    {
        public:
            virtual ~Module() = 0;
    };
}

#endif
