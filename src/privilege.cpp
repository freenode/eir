#include "privilege.h"

#include <set>

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>

using namespace eir;

namespace paludis
{
    template <>
    struct Implementation<PrivilegeSet>
    {
        std::set<std::string> privs;
    };
}

PrivilegeSet::iterator PrivilegeSet::begin()
{
    return _imp->privs.begin();
}

PrivilegeSet::iterator PrivilegeSet::end()
{
    return _imp->privs.end();
}

bool PrivilegeSet::has_privilege(std::string p)
{
    return _imp->privs.find(p) != _imp->privs.end();
}

void PrivilegeSet::add_privilege(std::string p)
{
    _imp->privs.insert(p);
}

void PrivilegeSet::clear()
{
    _imp->privs.clear();
}

PrivilegeSet::PrivilegeSet()
    : paludis::PrivateImplementationPattern<PrivilegeSet>(new paludis::Implementation<PrivilegeSet>())
{
}

PrivilegeSet::~PrivilegeSet()
{
}

template class paludis::WrappedForwardIterator<eir::PrivilegeSet::PrivilegeIteratorTag, const std::string>;
