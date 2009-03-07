#include "supported.h"
#include "exceptions.h"
#include "handler.h"

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <cstdlib>

using namespace eir;
using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<ISupport> : public CommandHandlerBase<Implementation<ISupport> >
    {
        std::set<std::string> simple_tokens;
        std::map<std::string, std::string> kv_tokens;
        int _max_modes;

        std::string _list_modes;
        std::string _simple_modes;
        std::string _oneparam_modes;
        std::string _oneparam_modes_2;

        std::string _prefixes, _prefix_modes;

        std::string _chantypes;

        void _populate(const Message *m);
        void _populate_prefix_modes(std::string);
        void _populate_chanmodes(std::string);

        CommandHolder _handler_id;

        Implementation(Bot *b)
        {
            _handler_id = add_handler(filter_command("005").from_bot(b), &Implementation<ISupport>::_populate);
        }
        ~Implementation()
        {
        }
    };
}

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
    return _imp->_chantypes.find(name[0]) != std::string::npos;
}

ISupport::ModeType ISupport::get_mode_type(char mode_letter) const
{
    if(_imp->_list_modes.find(mode_letter) != std::string::npos)
        return list_mode;
    if(_imp->_simple_modes.find(mode_letter) != std::string::npos)
        return simple_mode;
    if(_imp->_oneparam_modes.find(mode_letter) != std::string::npos)
        return oneparam_mode;
    if(_imp->_oneparam_modes_2.find(mode_letter) != std::string::npos)
        return oneparam2_mode;
    if(_imp->_prefix_modes.find(mode_letter) != std::string::npos)
        return prefix_mode;
    return unknown_mode;
}

void Implementation<ISupport>::_populate(const eir::Message *m)
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
            else if (name == "MODES")
                _max_modes = atoi(value.c_str());

            kv_tokens[name] = value;
        }
        else
        {
            simple_tokens.insert(simple_tokens.end(), token);
        }
    }
}

void Implementation<ISupport>::_populate_prefix_modes(std::string value)
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
    std::string::size_type i = _imp->_prefixes.find(m);
    if (i == std::string::npos)
        return 0;
    return _imp->_prefix_modes[i];
}

char ISupport::get_mode_prefix(char m) const
{
    std::string::size_type i = _imp->_prefix_modes.find(m);
    if (i == std::string::npos)
        return 0;
    return _imp->_prefixes[i];
}

bool ISupport::is_mode_prefix(char p) const
{
    return _imp->_prefixes.find(p) != std::string::npos;
}

void Implementation<ISupport>::_populate_chanmodes(std::string value)
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

ISupport::simple_iterator ISupport::begin_simple_tokens() const
{
    return _imp->simple_tokens.begin();
}

ISupport::simple_iterator ISupport::end_simple_tokens() const
{
    return _imp->simple_tokens.end();
}

ISupport::simple_iterator ISupport::find_simple_token(std::string s) const
{
    return _imp->simple_tokens.find(s);
}

bool ISupport::supports(std::string s) const
{
    return _imp->simple_tokens.find(s) != _imp->simple_tokens.end();
}

ISupport::kv_iterator ISupport::begin_kv() const
{
    return _imp->kv_tokens.begin();
}

ISupport::kv_iterator ISupport::end_kv() const
{
    return _imp->kv_tokens.end();
}

ISupport::kv_iterator ISupport::find_kv(std::string s) const
{
    return _imp->kv_tokens.find(s);
}

std::string ISupport::list_modes() const { return _imp->_list_modes; }
std::string ISupport::simple_modes() const { return _imp->_simple_modes; }
std::string ISupport::oneparam_modes() const { return _imp->_oneparam_modes; }
std::string ISupport::prefix_modes() const { return _imp->_prefix_modes; }

int ISupport::max_modes() const { return _imp->_max_modes; }

ISupport::ISupport(Bot *b)
    : PrivateImplementationPattern<ISupport>(new Implementation<ISupport>(b))
{
}

ISupport::~ISupport()
{
}

