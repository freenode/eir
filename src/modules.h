#ifndef modules_h
#define modules_h

#include <string>
#include <map>
#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>

namespace eir {

    struct ModuleError : public std::exception
    {
        std::string m;
        virtual const char *what() const throw ()
        {
            return m.c_str();
        }
        ModuleError(const char *s) : m(s) { }
        ModuleError(const std::string & s) : m(s) { }
        ~ModuleError() throw() { }
    };

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
