#include "eir.h"

#include <functional>

using namespace eir;
using namespace std::tr1::placeholders;

struct Operer : CommandHandlerBase<Operer>, Module
{
    void oper(const Message *m)
    {
        Bot::SettingsIterator it = m->bot->find_setting("oper_pass");
        if (it == m->bot->end_settings())
            return;

        std::string pass = it->second;

        std::string user(m->bot->nick());
        it = m->bot->find_setting("oper_name");
        if(it != m->bot->end_settings())
            user = (std::string)it->second;

        m->bot->send("OPER " + user + " " + pass);
    }

    void set_umode(const Message *m)
    {
        Bot::SettingsIterator it = m->bot->find_setting("oper_mode");
        if (it == m->bot->end_settings())
            return;

        std::string mode = it->second;

        m->bot->send("MODE " + m->bot->nick() + " " + mode);
    }

    CommandHolder connect_id, oper_up_id;

    Operer() {
        connect_id = add_handler(filter_command_type("001", sourceinfo::RawIrc), &Operer::oper);
        oper_up_id = add_handler(filter_command_type("381", sourceinfo::RawIrc), &Operer::set_umode);
    }
};

MODULE_CLASS(Operer)
