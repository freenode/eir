#include "bot.h"
#include "command.h"

using namespace eir;
using namespace std::tr1::placeholders;

struct command_dumper {
    void do_dump(const Message *m)
    {
        if (m->source.destination != m->bot->nick())
            return;
        if (m->args[0].substr(0, 8) == "isupport")
        {
            m->source.reply("simple modes: " + m->bot->supported()->simple_modes());
            m->source.reply("list modes: " + m->bot->supported()->list_modes());
            m->source.reply("oneparam modes: " + m->bot->supported()->oneparam_modes());
            m->source.reply("isupport tokens:");
            for(ISupport::simple_iterator it = m->bot->supported()->begin_simple_tokens(),
                    ite = m->bot->supported()->end_simple_tokens();
                    it != ite; ++it)
                m->source.reply("    " + *it);
            m->source.reply("key-value tokens:");
            for(ISupport::kv_iterator it = m->bot->supported()->begin_kv(),
                    ite = m->bot->supported()->end_kv();
                    it != ite; ++it)
                m->source.reply("    " + it->first + " = " + it->second);

        }
    }
    command_dumper() { _id = CommandRegistry::get_instance()->add_handler("PRIVMSG", std::tr1::bind(std::tr1::mem_fn(&command_dumper::do_dump), this, _1)); }
    ~command_dumper() { CommandRegistry::get_instance()->remove_handler(_id); }
    CommandRegistry::id _id;
};

command_dumper cd;

