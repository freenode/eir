#if 0
#include "bot.h"
#include "handler.h"

using namespace eir;
using namespace std::tr1::placeholders;

struct Dumper : public CommandHandlerBase<Dumper>
{
    void do_dump(const Message *m)
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
    Dumper() { _id = add_handler("isupport", sourceinfo::IrcCommand, &Dumper::do_dump); }
    CommandHolder _id;
} cd;

#endif
