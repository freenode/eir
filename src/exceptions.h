#ifndef exceptions_h
#define exceptions_h

#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>

#define assert(x) if(!x) throw paludis::InternalError(PALUDIS_HERE, "Assertion failed: "#x)

namespace eir {
    using paludis::Context;

    struct Exception : public paludis::Exception
    {
        private:
            bool _fatal;
        public:
            Exception(std::string m, bool fatal = false)
                : paludis::Exception(m), _fatal(fatal)
            { }

            bool fatal() const { return _fatal; }
    };

    struct DieException : public Exception
    {
        DieException(std::string who) : Exception("Shut down by " + who, true)
        { }
    };

    struct RestartException : public Exception
    {
        RestartException() : Exception("Restarting...", true)
        { }
    };

    struct NotConnectedException : public Exception
    {
        NotConnectedException() : Exception("Not connected to server")
        { }
    };

    struct ConnectionError : public Exception
    {
        ConnectionError(std::string s) : Exception(s)
        { }
    };

    struct ModuleError : public Exception
    {
        ModuleError(std::string s) : Exception(s, false) { }
    };

    struct ConfigurationError : public Exception
    {
        ConfigurationError(std::string s) : Exception(s) { }
    };

    struct InternalError : public Exception
    {
        InternalError(std::string s) : Exception(s) { }
    };

}

#endif
