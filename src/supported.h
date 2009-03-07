#ifndef supported_h
#define supported_h

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <string>
#include <set>
#include <map>

namespace eir
{
    struct Message;
    class Bot;

    class ISupport : private paludis::InstantiationPolicy<ISupport,paludis::instantiation_method::NonCopyableTag>,
                     private paludis::PrivateImplementationPattern<ISupport>
    {
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

            simple_iterator begin_simple_tokens() const;
            simple_iterator end_simple_tokens() const;
            simple_iterator find_simple_token(std::string s) const;

            bool supports(std::string s) const;

            typedef std::map<std::string, std::string>::const_iterator kv_iterator;

            kv_iterator begin_kv() const;
            kv_iterator end_kv() const;
            kv_iterator find_kv(std::string s) const;

            std::pair<bool, std::string> get_value(std::string s) const;

            int max_modes() const;
            std::string list_modes() const;
            std::string simple_modes() const;
            std::string oneparam_modes() const;
            std::string prefix_modes() const;

            char get_prefix_mode(char prefix) const;
            char get_mode_prefix(char mode) const;

            ModeType get_mode_type(char c) const;

            bool is_mode_prefix(char c) const;

            bool mode_has_param(char modeletter, bool adding = true) const;

            bool is_channel_name(std::string) const;

            ISupport(Bot*);
            ~ISupport();
    };
}

#endif
