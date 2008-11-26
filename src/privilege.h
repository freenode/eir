#ifndef privilege_h
#define privilege_h

#include <string>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace eir
{
    class PrivilegeSet : public paludis::PrivateImplementationPattern<PrivilegeSet>
    {
        public:
            struct PrivilegeIteratorTag;
            typedef paludis::WrappedForwardIterator<PrivilegeIteratorTag, const std::string> iterator;

            iterator begin();
            iterator end();

            bool has_privilege(std::string);
            void add_privilege(std::string);
            void clear();

            PrivilegeSet();
            ~PrivilegeSet();
    };
}

#endif
