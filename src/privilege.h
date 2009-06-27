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
            typedef paludis::WrappedForwardIterator<PrivilegeIteratorTag,
                                                    const std::pair<std::string, std::string> > iterator;

            iterator begin();
            iterator end();

            // One-argument forms are for global privilege; two-argument forms for channel privs.
            bool has_privilege(std::string);
            void add_privilege(std::string);

            bool has_privilege(std::string, std::string);
            void add_privilege(std::string, std::string);

            void clear();

            PrivilegeSet();
            ~PrivilegeSet();
    };
}

#endif
