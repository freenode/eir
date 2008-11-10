#ifndef client_h
#define client_h

#include "pimpl.h"
#include <string>

#include <paludis/util/wrapped_forward_iterator.hh>

namespace eir
{
    struct Client : private util::Pimpl<Client>
    {
        const std::string& nick() const;
        const std::string& user() const;
        const std::string& host() const;

        struct AttributeIteratorTag;
        typedef paludis::WrappedForwardIterator<AttributeIteratorTag,
                        std::pair<const std::string, std::string> > AttributeIterator;

        AttributeIterator attr_begin();
        AttributeIterator attr_end();

        const std::string& attr(const std::string &) const;
        void set_attr(const std::string &, const std::string &);

        int channelcount;

        Client(std::string, std::string, std::string);

        void join_chan(Channel *);
        void leave_chan(Channel *);
    };

    struct Channel : private util::Pimpl<Channel>
    {
        const std::string& name();

        struct MemberIteratorTag;
        typedef paludis::WrappedForwardIterator<MemberIteratorTag, Client * const> MemberIterator;
        MemberIterator begin_members();
        MemberIterator end_members();

        void add_member(struct Client *);
        bool remove_member(struct Client *);

        Channel(std::string);
        ~Channel();
    };
}

#endif
