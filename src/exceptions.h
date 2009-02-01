#ifndef exceptions_h
#define exceptions_h

#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>

#define assert(x) if(!x) throw paludis::InternalError(PALUDIS_HERE, "Assertion failed: "#x)

namespace eir {
    using paludis::Context;

    struct DieException : public paludis::Exception
    {
        DieException(std::string who) : paludis::Exception("Shut down by " + who)
        { }
    };

    struct RestartException : public paludis::Exception
    {
        RestartException() : paludis::Exception("Restarting...")
        { }
    };

    struct NotConnectedException : public paludis::Exception
    {
        NotConnectedException() : paludis::Exception("Not connected to server")
        { }
    };

}

#endif
