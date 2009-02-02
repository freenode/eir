#include "eir.h"

#include "handler.h"

using namespace std::tr1::placeholders;
using namespace eir;

struct ModeParser : public CommandHandlerBase<ModeParser>
{
    void check_mode_changes(const Message *m)
    {
        if (m->args.size() < 2)
            return;

        Message m2(*m, sourceinfo::Internal, "mode " + m->args[1]);
        m2.args.push_back(m->args[0]);
        if (m->args.size() >= 3)
            m2.args.push_back(m->args[2]);

        CommandRegistry::get_instance()->dispatch(&m2);

        if (m->bot->supported()->get_mode_type(m->args[1][0]) == ISupport::prefix_mode &&
                m->args.size() >= 3)
        {
            // Prefix mode -- update the client's Membership list.
            Client::ptr c = m->bot->find_client(m->args[2]);
            if (!c)
                return;

            Membership::ptr mem = c->find_membership(m->source.destination);
            if (!mem)
                return;

            if (!mem->has_mode(m->args[1][0]))
                mem->modes += m->args[1][0];
        }
    }

    void parse_mode(const Message *m)
    {
        if (! m->bot->supported()->is_channel_name(m->source.destination))
            return;

        std::vector<std::string>::const_iterator nextarg = m->args.begin();
        std::string modes = *nextarg++;

        int dir = 0;
        for ( std::string::iterator ch = modes.begin(); ch != modes.end(); ++ch)
        {
            if (*ch == '+')
                dir = 1;
            else if (*ch == '-')
                dir = -1;
            else
            {
                Message m2(*m, sourceinfo::Internal, "mode_change");

                m2.args.push_back(dir > 0 ? "add" : "remove");
                m2.args.push_back(std::string(1, *ch));
                m2.raw = std::string("mode_change ") + m2.args[0] + " " + m2.args[1];

                if (m->bot->supported()->mode_has_param(*ch, dir>0))
                {
                    m2.raw += " " + *nextarg;
                    m2.args.push_back(*nextarg++);
                }

                CommandRegistry::get_instance()->dispatch(&m2);

                check_mode_changes(&m2);
            }
        }
    }

    CommandRegistry::id mode_id;

    ModeParser()
    {
        mode_id = add_handler("MODE", sourceinfo::RawIrc, &ModeParser::parse_mode);
    }
    ~ModeParser()
    {
        remove_handler(mode_id);
    }
} mode_parser;

