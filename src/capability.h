#ifndef capability_h
#define capability_h

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <string>

namespace eir
{
    class Bot;

    class Capabilities : private paludis::InstantiationPolicy<Capabilities, paludis::instantiation_method::NonCopyableTag>,
                         private paludis::PrivateImplementationPattern<Capabilities>
    {
        public:
            struct CapTokenIteratorTag;
            typedef paludis::WrappedForwardIterator<CapTokenIteratorTag, std::string const> iterator;

            iterator begin_available() const;
            iterator end_available() const;
            bool is_available(std::string cap) const;

            iterator begin_enabled() const;
            iterator end_enabled() const;
            bool is_enabled(std::string cap) const;

            void request(std::string cap);

            /*
             * If a CAP handler (e.g. SASL) needs to keep cap negotiation open to do its own processing,
             * then it should call hold() in response to the cap being enabled and finish() once it has
             * finished. Such a hold will time out in a few seconds.
             */
            void hold();
            void finish();

            Capabilities(Bot * b);
            ~Capabilities();
    };
}

#endif
