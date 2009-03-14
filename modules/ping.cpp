#include "eir.h"
#include "handler.h"

#include <functional>
#include <iostream>

using namespace eir;
using namespace std::tr1::placeholders;

struct Ponger : CommandHandlerBase<Ponger>, Module
{
    CommandHolder _id;

    void pong(const eir::Message *m)
    {
        std::string response("PONG :" + m->source.destination);
        m->bot->send(response);
    }

    Ponger() {
        _id = add_handler(filter_command_type("PING", sourceinfo::RawIrc), &Ponger::pong);
    }
};

MODULE_CLASS(Ponger)
