#include "eir.h"

#include "handler.h"

#include <paludis/util/tokeniser.hh>

using namespace std::tr1::placeholders;
using namespace eir;

namespace {
    void notice_to(Bot *b, std::string dest, std::string text)
    {
        b->send("NOTICE " + dest + " :" + text);
    }
}

struct BotCommandHandler : public CommandHandlerBase<BotCommandHandler>
{
    CommandHolder _id;

    void handle_privmsg(const Message *m)
    {
        std::string line, reply_dest;
        if (m->source.destination == m->bot->nick())
        {
            line = m->args[0];
            reply_dest = m->source.name;
        }
        else if (m->bot->get_setting("command_chars").find(m->args[0][0]) != std::string::npos)
        {
            line = m->args[0].substr(1);
            reply_dest = m->source.destination;
        }
        else if (m->args[0].substr(0, m->args[0].find_first_of(",: ")) == m->bot->nick())
        {
            line = m->args[0].substr(m->args[0].find_first_of(",: ") + 1);
            reply_dest = m->source.destination;
        }
        else
            return;

        std::list<std::string> tokens;
        paludis::tokenise_whitespace(line, std::back_inserter(tokens));

        if(tokens.empty())
            return;

        Message m2(*m, sourceinfo::IrcCommand, *tokens.begin());

        tokens.pop_front();
        std::copy(tokens.begin(), tokens.end(), std::back_inserter(m2.args));

        m2.raw = line;

        CommandRegistry::get_instance()->dispatch(&m2);
    }

    BotCommandHandler()
    {
        _id = add_handler("PRIVMSG", sourceinfo::RawIrc, &BotCommandHandler::handle_privmsg);
    }
} bot_command_handler;

