#ifndef settings_h
#define settings_h

#include "value.h"

#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace eir
{
    class GlobalSettingsManager : public paludis::PrivateImplementationPattern<GlobalSettingsManager>,
                                  public paludis::InstantiationPolicy<GlobalSettingsManager, paludis::instantiation_method::SingletonTag>
    {
        public:
            struct IteratorTag;
            typedef paludis::WrappedForwardIterator<IteratorTag,
                                        std::pair<const std::string, Value> > iterator;
            iterator begin();
            iterator end();

            iterator find(std::string name);

            Value& get(std::string name);
            Value get_with_default(std::string name, Value _default);

            bool add(std::string n, Value v);

            size_t remove(std::string n);
            void remove(iterator it);

            GlobalSettingsManager();
            ~GlobalSettingsManager();
    };
}

#endif
