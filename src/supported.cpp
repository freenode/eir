#include "supported.h"
#include "exceptions.h"
#include "message.h"

using namespace eir;

bool ISupport::mode_has_param(char mode_letter, bool adding /*= true*/) const
{
    if(_list_modes.find(mode_letter) != std::string::npos)
        return true;
    if(_simple_modes.find(mode_letter) != std::string::npos)
        return false;
    if(_oneparam_modes.find(mode_letter) != std::string::npos)
        return adding;
    if(_oneparam_modes_2.find(mode_letter) != std::string::npos)
        return true;
    return false;
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

        if((idx = token.find('=')) != std::string::npos)
        {
            std::string name = token.substr(0, idx);
            std::string value = token.substr(idx+1);
            if(name == "CHANMODES")
                _populate_chanmodes(value);
            else
                kv_tokens[name] = value;
        }
        else
        {
            simple_tokens.insert(simple_tokens.end(), token);
        }
    }
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

