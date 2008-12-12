#ifndef exceptions_h
#define exceptions_h

#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>

#define assert(x) if(!x) throw paludis::InternalError(PALUDIS_HERE, "Assertion failed: "#x)

namespace eir {
    using paludis::Context;
}

#endif
