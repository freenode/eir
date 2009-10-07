// This file shouldn't really exist.
// Unfortunately, type info for all the exception classes has to
// exist inside the main executable, or throwing an exception from
// one module that's caught in another will fail.

#include "exceptions.h"

using namespace eir;

namespace {
    void silly_dummy_function()
    {
        DieException a("");
        RestartException b;
        NotConnectedException c;
        ConnectionError d("");
        ModuleError e("");
        ConfigurationError f("");
        InternalError g("");
        StorageError h("");
        IOError i("");
        NotFoundError j("");
    }
}

