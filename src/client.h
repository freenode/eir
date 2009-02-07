#ifndef client_h
#define client_h

#include "privilege.h"

#include <string>
#include <tr1/memory>

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

namespace eir
{
    class Bot;

    struct Channel;
    typedef std::tr1::shared_ptr<Channel> ChannelPtr;
    struct Client;
    typedef std::tr1::shared_ptr<Client> ClientPtr;
    struct Membership;
    typedef std::tr1::shared_ptr<Membership> MembershipPtr;

    struct Client : private paludis::PrivateImplementationPattern<Client>,
                    private paludis::InstantiationPolicy<Client, paludis::instantiation_method::NonCopyableTag>,
                    public std::tr1::enable_shared_from_this<Client>
    {
        const std::string& nick() const;
        const std::string& user() const;
        const std::string& host() const;
        const std::string& nuh() const;

        void change_nick(std::string newnick);

        struct AttributeIteratorTag;
        typedef paludis::WrappedForwardIterator<AttributeIteratorTag,
                        std::pair<const std::string, std::string> > AttributeIterator;

        AttributeIterator attr_begin();
        AttributeIterator attr_end();

        std::string attr(const std::string &);
        void set_attr(const std::string &, const std::string &);

        Client(Bot *, std::string, std::string, std::string);
        ~Client();

        MembershipPtr join_chan(ChannelPtr);
        void leave_chan(ChannelPtr);
        void leave_chan(MembershipPtr);

        struct ChannelIteratorTag;
        typedef paludis::WrappedForwardIterator<ChannelIteratorTag, const MembershipPtr> ChannelIterator;
        ChannelIterator begin_channels();
        ChannelIterator end_channels();
        MembershipPtr find_membership(std::string channel);
        const MembershipPtr find_membership(std::string channel) const;

        PrivilegeSet& privs();

        typedef std::tr1::shared_ptr<Client> ptr;
    };

    struct Channel : private paludis::PrivateImplementationPattern<Channel>,
                     private paludis::InstantiationPolicy<Channel, paludis::instantiation_method::NonCopyableTag>,
                     public std::tr1::enable_shared_from_this<Channel>
    {
        const std::string& name();

        struct MemberIteratorTag;
        typedef paludis::WrappedForwardIterator<MemberIteratorTag, const MembershipPtr> MemberIterator;
        MemberIterator begin_members();
        MemberIterator end_members();

        bool add_member(MembershipPtr);
        bool remove_member(MembershipPtr);

        Channel(std::string);
        ~Channel();

        typedef std::tr1::shared_ptr<Channel> ptr;
    };

    struct Membership : private paludis::InstantiationPolicy<Membership, paludis::instantiation_method::NonCopyableTag>
    {
        Client::ptr client;
        Channel::ptr channel;

        std::string modes;

        bool has_mode(char m) { return modes.find(m) != std::string::npos; }

        typedef std::tr1::shared_ptr<Membership> ptr;

        Membership(Client::ptr cl, Channel::ptr ch)
            : client(cl), channel(ch)
        { }
    };
}

#endif
