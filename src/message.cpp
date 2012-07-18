#include "message.h"
#include "string_util.h"
#include "match.h"
#include "bot.h"

using namespace eir;

Filter::Filter()
    : matches(0), bot(0), sourcetype(0)
{
}

Filter& Filter::is_command(std::string c)
{
    matches |= match_command;
    commandname = c;
    return *this;
}

Filter& Filter::source_type(unsigned int t)
{
    matches |= match_source_type;
    sourcetype = t;
    return *this;
}

Filter& Filter::source_named(std::string n)
{
    matches |= match_source_name;
    source = n;
    return *this;
}

Filter& Filter::from_bot(Bot *b)
{
    matches |= match_bot;
    bot = b;
    return *this;
}

Filter& Filter::in_private()
{
    matches |= match_private;
    return *this;
}

Filter& Filter::in_channel(std::string c)
{
    matches |= match_in_channel;
    channel = c;
    return *this;
}

Filter& Filter::requires_privilege(std::string p)
{
    matches |= match_privilege;
    privilege = p;
    return *this;
}

Filter& Filter::or_config()
{
    matches |= match_config_overrides;
    return *this;
}

bool Filter::match(const Message *m) const
{
    if (matches & match_source_type && 0 == (sourcetype & m->source.type))
        return false;
    if (matches & match_command && ! cistring::equal(commandname, m->command))
        return false;
    if (matches & match_bot && bot != m->bot)
        return false;
    if (matches & match_config_overrides && m->source.type == sourceinfo::ConfigFile)
        return true;
    if (matches & match_privilege && ! ( m->source.client && m->source.client->privs().has_privilege(privilege)))
        return false;
    if (matches & match_private && m->source.destination != m->bot->nick())
        return false;
    if (matches & match_in_channel && m->source.destination != channel)
        return false;
    if (matches & match_source_name && ! ::match(source, m->source.name))
        return false;

    return true;
}

