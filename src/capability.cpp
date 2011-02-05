#include "capability.h"
#include "eir.h"

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/join.hh>

#include <set>
#include <vector>
#include <algorithm>

using namespace eir;
using paludis::Implementation;

namespace paludis
{
    template <>
    struct Implementation<Capabilities> : CommandHandlerBase<Implementation<Capabilities>>
    {
        Bot *_bot;

        std::set<std::string> caps_available, caps_enabled;
        // caps_to_request is the set of tokens that we've been asked to request
        // caps_requested is those we've actually sent to the ircd
        std::set<std::string> caps_to_request, caps_requested;

        int cap_end_hold;
        EventManager::id timeout_event_id;

        void on_connect(const Message * m);
        void on_cap_reply(const Message * m);

        CommandHolder connect_holder, cap_holder;

        Implementation(Bot *b) :
            _bot(b),
            cap_end_hold(0),
            timeout_event_id(0)
        {
            connect_holder = add_handler(filter_command_type("on_connect", sourceinfo::Internal).from_bot(b),
                    &Implementation<Capabilities>::on_connect);
            cap_holder = add_handler(filter_command_type("CAP", sourceinfo::RawIrc).from_bot(b),
                    &Implementation<Capabilities>::on_cap_reply);
        }
    };
}

void Implementation<Capabilities>::on_connect(const Message *m)
{
    m->bot->send("CAP LS");
}

void Implementation<Capabilities>::on_cap_reply(const Message *m)
{
    if (m->args.size() < 2)
        return;

    std::string subcommand = m->args[0];

    std::vector<std::string> cap_tokens;
    paludis::tokenise_whitespace(m->args[1], std::back_inserter(cap_tokens));

    if (subcommand == "LS") {

        for (auto it = cap_tokens.begin(); it != cap_tokens.end(); ++it)
        {
            caps_available.insert(*it);

            if (caps_to_request.find(*it) != caps_to_request.end())
                caps_requested.insert(*it);
        }

        if (caps_requested.size())
        {
            std::string request = paludis::join(caps_requested.begin(), caps_requested.end(), std::string(" "));
            m->bot->send("CAP REQ :" + request);
        }
    } else if (subcommand == "ACK") {
        for (auto it = cap_tokens.begin(); it != cap_tokens.end(); ++it)
        {
            caps_enabled.insert(*it);
            caps_requested.erase(*it);

            Message cap_enabled(m->bot, "cap_enabled", sourceinfo::Internal);
            cap_enabled.args.push_back(*it);
            CommandRegistry::get_instance()->dispatch(&cap_enabled);
        }
    } else if (subcommand == "NAK") {
        for (auto it = cap_tokens.begin(); it != cap_tokens.end(); ++it)
        {
            caps_requested.erase(*it);
        }
    }

    if (caps_requested.empty())
    {
        if (cap_end_hold == 0)
            m->bot->send("CAP END");
        else
        {
            Bot *b = m->bot;
            timeout_event_id = EventManager::get_instance()->add_event(time(NULL) + 5, [b](){ b->send("CAP END"); });
        }
    }
}

Capabilities::Capabilities(Bot *b)
    : PrivateImplementationPattern<Capabilities>(new paludis::Implementation<Capabilities>(b))
{
}

Capabilities::~Capabilities()
{
}

void Capabilities::hold()
{
    ++_imp->cap_end_hold;
}

void Capabilities::finish()
{
    if (--_imp->cap_end_hold <= 0)
    {
        _imp->_bot->send("CAP END");
        EventManager::get_instance()->remove_event(_imp->timeout_event_id);
        _imp->timeout_event_id = 0;
    }
}

Capabilities::iterator Capabilities::begin_available() const
{
    return _imp->caps_available.begin();
}

Capabilities::iterator Capabilities::end_available() const
{
    return _imp->caps_available.end();
}

bool Capabilities::is_available(std::string cap) const
{
    return _imp->caps_available.find(cap) != _imp->caps_available.end();
}


Capabilities::iterator Capabilities::begin_enabled() const
{
    return _imp->caps_enabled.begin();
}

Capabilities::iterator Capabilities::end_enabled() const
{
    return _imp->caps_enabled.end();
}

bool Capabilities::is_enabled(std::string cap) const
{
    return _imp->caps_enabled.find(cap) != _imp->caps_enabled.end();
}

void Capabilities::request(std::string cap)
{
    _imp->caps_to_request.insert(cap);
}

