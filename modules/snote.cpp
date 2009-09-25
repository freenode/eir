#include "eir.h"

#include <functional>

using namespace eir;
using namespace std::tr1::placeholders;

struct SnoteHandler : CommandHandlerBase<SnoteHandler>, Module
{
    void on_notice(const Message *m)
    {
        // Shouldn't happen, but check anyway.
        if (m->args.empty())
            return;

        // We're only interested in notices from a server
        if (m->source.raw.find(".") == std::string::npos)
            return;

        std::string text = m->args[0];
        std::string prefix = "*** Notice -- ";
        if (text.compare(0, prefix.size(), prefix) != 0)
            return;
        text = text.substr(prefix.size());

        Message m2(*m, "incoming_snote", sourceinfo::Internal);
        m2.args.push_back(text);
        CommandRegistry::get_instance()->dispatch(&m2);
    }

    CommandHolder _id;

    SnoteHandler()
    {
        _id = add_handler(filter_command_type("notice", sourceinfo::RawIrc), &SnoteHandler::on_notice);
    }
};

MODULE_CLASS(SnoteHandler)

