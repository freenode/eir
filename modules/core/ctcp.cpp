#include "eir.h"

#include <paludis/util/tokeniser.hh>

using namespace eir;

struct CtcpHandler : CommandHandlerBase<CtcpHandler>, Module
{
    void on_notice(const Message *m)
    {
        // Shouldn't happen, but check anyway.
        if (m->args.empty())
            return;

        std::string text = m->args[0];

        if (text[0] != '\001')
            return;

        text = text.substr(1);
        std::string::size_type pos = text.rfind('\001');
        if (pos != std::string::npos)
            text.erase(pos);

        pos = text.find(' ');
        std::string command = text.substr(0, pos);
        std::string arg = text.substr(pos + 1);

        Message m2(*m, "ctcp_reply", sourceinfo::Internal);
        m2.args.push_back(command);
        m2.args.push_back(arg);
        CommandRegistry::get_instance()->dispatch(&m2);
    }

    CommandHolder _id;

    CtcpHandler()
    {
        _id = add_handler(filter_command_type("notice", sourceinfo::RawIrc), &CtcpHandler::on_notice);
    }
};

MODULE_CLASS(CtcpHandler)

