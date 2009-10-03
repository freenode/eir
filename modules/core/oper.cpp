#include "eir.h"

#include <functional>

using namespace eir;
using namespace std::tr1::placeholders;

struct Operer : CommandHandlerBase<Operer>, Module
{
    void oper(const Message *m)
    {
        Bot::SettingsIterator it = m->bot->find_setting("oper_pass");
        if(it == m->bot->end_settings())
            return;

        std::string pass = it->second;

        std::string user(m->bot->nick());
        it = m->bot->find_setting("oper_name");
        if(it != m->bot->end_settings())
            user = (std::string)it->second;

        m->bot->send("OPER " + user + " " + pass);
    }

    CommandHolder _id;

    Operer() {
        _id = add_handler(filter_command_type("001", sourceinfo::RawIrc), &Operer::oper);
    }
};

MODULE_CLASS(Operer)
