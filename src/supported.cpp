#include "supported.h"
#include "exceptions.h"
#include "message.h"

using namespace eir;

bool ISupport::mode_has_param(char mode_letter, bool adding /*= true*/) const
{
    switch(get_mode_type(mode_letter))
    {
        case list_mode:
        case oneparam2_mode:
        case prefix_mode:
            return true;
        case simple_mode:
        case unknown_mode:
            return false;
        case oneparam_mode:
            return adding;
    }
    return false;
}

bool ISupport::is_channel_name(std::string name) const
{
    return _chantypes.find(name[0]) != std::string::npos;
}

ISupport::ModeType ISupport::get_mode_type(char mode_letter) const
{
    if(_list_modes.find(mode_letter) != std::string::npos)
        return list_mode;
    if(_simple_modes.find(mode_letter) != std::string::npos)
        return simple_mode;
    if(_oneparam_modes.find(mode_letter) != std::string::npos)
        return oneparam_mode;
    if(_oneparam_modes_2.find(mode_letter) != std::string::npos)
        return oneparam2_mode;
    if(_prefix_modes.find(mode_letter) != std::string::npos)
        return prefix_mode;
    return unknown_mode;
}

void ISupport::_populate(const eir::Message *m)
{
    Context c("Populating ISUPPORT values");

    std::vector<std::string>::const_iterator it, ite;
    for(it = m->args.begin(), ite = m->args.end(); it != ite; ++it)
    {
        std::string token = *it;
        std::string::size_type idx;

        if (token.find("are supported") != std::string::npos)
            continue;

        if ((idx = token.find('=')) != std::string::npos)
        {
            std::string name = token.substr(0, idx);
            std::string value = token.substr(idx+1);

            if (name == "CHANMODES")
                _populate_chanmodes(value);
            else if (name == "CHANTYPES")
                _chantypes = value;
            else if (name == "PREFIX")
                _populate_prefix_modes(value);

            kv_tokens[name] = value;
        }
        else
        {
            simple_tokens.insert(simple_tokens.end(), token);
        }
    }
}

void ISupport::_populate_prefix_modes(std::string value)
{
    if (value[0] != '(')
        return;

    std::string::size_type idx = value.find(')');

    if (idx == std::string::npos)
        return;

    _prefix_modes = value.substr(1, idx-1);
    _prefixes = value.substr(idx + 1);
}

char ISupport::get_prefix_mode(char m) const
{
    std::string::size_type i = _prefixes.find(m);
    if (i == std::string::npos)
        return 0;
    return _prefix_modes[i];
}

char ISupport::get_mode_prefix(char m) const
{
    std::string::size_type i = _prefix_modes.find(m);
    if (i == std::string::npos)
        return 0;
    return _prefixes[i];
}

bool ISupport::is_mode_prefix(char p) const
{
    return _prefixes.find(p) != std::string::npos;
}

void ISupport::_populate_chanmodes(std::string value)
{
    Context c("Populating channel mode types from " + value);
    std::string::size_type begin = 0, end = 0;
    end = value.find(',');
    _list_modes = value.substr(begin, end - begin);
    begin = end + 1;
    end = value.find(',', begin);
    _oneparam_modes_2 = value.substr(begin, end - begin);
    begin = end + 1;
    end = value.find(',', begin);
    _oneparam_modes = value.substr(begin, end - begin);
    begin = end + 1;
    _simple_modes = value.substr(begin);
}

using namespace std::tr1::placeholders;

ISupport::ISupport()
{
    _handler_id = CommandRegistry::get_instance()->add_handler("005", std::tr1::bind(&ISupport::_populate, this, _1));
}

ISupport::~ISupport()
{
    CommandRegistry::get_instance()->remove_handler(_handler_id);
}

