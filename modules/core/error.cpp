#include "eir.h"

#include "handler.h"

using namespace eir;

struct ErrorHandler : CommandHandlerBase<ErrorHandler>, Module
{
    void error(const Message *m)
    {
        // ERROR is a strange command without a destination.
        // What we parse as the destination string is actually the error string.
        throw DisconnectedException(m->source.destination);
    }
    CommandHolder id;

    ErrorHandler()
    {
        id = add_handler(filter_command_type("ERROR", sourceinfo::RawIrc), &ErrorHandler::error);
    }
};

MODULE_CLASS(ErrorHandler)
