#ifndef client_h
#define client_h

#include "privilege.h"

#include <string>
#include <tr1/memory>

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

namespace eir
{
    struct Channel;
    typedef std::tr1::shared_ptr<Channel> ChannelPtr;

    struct Client : private paludis::PrivateImplementationPattern<Client>,
                    public std::tr1::enable_shared_from_this<Client>
    {
        const std::string& nick() const;
        const std::string& user() const;
        const std::string& host() const;

        struct AttributeIteratorTag;
        typedef paludis::WrappedForwardIterator<AttributeIteratorTag,
                        std::pair<const std::string, std::string> > AttributeIterator;

        AttributeIterator attr_begin();
        AttributeIterator attr_end();

        std::string attr(const std::string &);
        void set_attr(const std::string &, const std::string &);

        Client(std::string, std::string, std::string);
        ~Client();

        void join_chan(ChannelPtr);
        void leave_chan(ChannelPtr);

        struct ChannelIteratorTag;
        typedef paludis::WrappedForwardIterator<ChannelIteratorTag, const ChannelPtr> ChannelIterator;
        ChannelIterator begin_channels();
        ChannelIterator end_channels();

        PrivilegeSet& privs();

        typedef std::tr1::shared_ptr<Client> ptr;
    };

    struct Channel : private paludis::PrivateImplementationPattern<Channel>,
                     public std::tr1::enable_shared_from_this<Channel>
    {
        const std::string& name();

        struct MemberIteratorTag;
        typedef paludis::WrappedForwardIterator<MemberIteratorTag, const Client::ptr> MemberIterator;
        MemberIterator begin_members();
        MemberIterator end_members();

        bool add_member(Client::ptr);
        bool remove_member(Client::ptr);

        Channel(std::string);
        ~Channel();

        typedef std::tr1::shared_ptr<Channel> ptr;
    };
}

#endif
