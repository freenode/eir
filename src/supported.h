#ifndef supported_h
#define supported_h

#include "command.h"
#include <paludis/util/instantiation_policy.hh>
#include <string>
#include <set>
#include <map>

namespace eir
{
    struct Message;

    class ISupport : private paludis::InstantiationPolicy<ISupport,paludis::instantiation_method::NonCopyableTag>
    {
        private:
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

            CommandRegistry::id _handler_id;

        public:
            typedef std::set<std::string>::const_iterator simple_iterator;

            enum ModeType {
                simple_mode,
                list_mode,
                oneparam_mode,
                oneparam2_mode,
                prefix_mode,
                unknown_mode
            };

            simple_iterator begin_simple_tokens() const { return simple_tokens.begin(); }
            simple_iterator end_simple_tokens() const { return simple_tokens.end(); }
            simple_iterator find_simple_token(std::string s) const { return simple_tokens.find(s); }

            bool supports(std::string s) const { return simple_tokens.find(s) != simple_tokens.end(); }

            typedef std::map<std::string, std::string>::const_iterator kv_iterator;

            kv_iterator begin_kv() const { return kv_tokens.begin(); }
            kv_iterator end_kv() const { return kv_tokens.end(); }
            kv_iterator find_kv(std::string s) const { return kv_tokens.find(s); }

            std::pair<bool, std::string> get_value(std::string s) const
              { kv_iterator i=find_kv(s);
                return i == end_kv() ? std::make_pair(false, std::string("")) : std::make_pair(true, i->second); }

            int max_modes() const { return _max_modes; }
            std::string list_modes() const { return _list_modes; }
            std::string simple_modes() const { return _simple_modes; }
            std::string oneparam_modes() const { return _oneparam_modes; }
            std::string prefix_modes() const { return _prefix_modes; }

            char get_prefix_mode(char prefix) const;
            char get_mode_prefix(char mode) const;

            ModeType get_mode_type(char c) const;

            bool mode_has_param(char modeletter, bool adding = true) const;

            bool is_channel_name(std::string) const;

            ISupport();
            ~ISupport();
    };
}

#endif
