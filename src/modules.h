#ifndef modules_h
#define modules_h

#include <string>
#include <map>
#include "exceptions.h"
#include <paludis/util/instantiation_policy.hh>

namespace eir {

    class ModuleRegistry : public paludis::InstantiationPolicy<ModuleRegistry, paludis::instantiation_method::SingletonTag>
    {
        public:
            void load(std::string) throw(ModuleError);
            bool unload(std::string);

            bool is_loaded(std::string);

            /*
            template <typename T>
                T getsym(std::string, std::string);
            */

        private:
            typedef void *id;

            std::map<std::string, id> _modules;
    };
}

#endif
