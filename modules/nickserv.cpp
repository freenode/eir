#include "eir.h"

#include <functional>
#include <iostream>

using namespace eir;
using namespace std::tr1::placeholders;

struct Identifier : CommandHandlerBase<Identifier>, Module
{
    void identify(const Message *m)
    {
        Bot::SettingsIterator it = m->bot->find_setting("nickserv_pass");
        if(it == m->bot->end_settings())
            return;

        std::string pass = it->second;

        std::string user("");
        it = m->bot->find_setting("nickserv_user");
        if(it != m->bot->end_settings())
            user = it->second;

        m->bot->send("NS IDENTIFY " + user + " " + pass);
    }

    CommandHolder _id;

    Identifier() {
        _id = add_handler(filter_command_type("001", sourceinfo::RawIrc), &Identifier::identify);
    }
};

MODULE_CLASS(Identifier)
