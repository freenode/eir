#include "eir.h"

#include <functional>
#include <iostream>

using namespace std::tr1::placeholders;

struct Ponger
{
    eir::CommandRegistry::id _id;

    void pong(const eir::Message *m)
    {
        std::string response("PONG :" + m->source.destination);
        std::cerr << "--> " << response << std::endl;
        m->bot->send(response);
    }

    Ponger() {
        _id = eir::CommandRegistry::get_instance()->add_handler("PING", std::tr1::bind(std::tr1::mem_fn(&Ponger::pong), this, _1));
    }
    ~Ponger() {
        eir::CommandRegistry::get_instance()->remove_handler(_id);
    }
};

Ponger p;
