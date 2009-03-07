#ifndef command_h
#define command_h

#include <map>
#include <list>
#include <tr1/functional>
#include "message.h"
#include <paludis/util/instantiation_policy.hh>

namespace eir
{
    class CommandRegistry :
        public paludis::InstantiationPolicy<CommandRegistry, paludis::instantiation_method::SingletonTag>,
        public paludis::PrivateImplementationPattern<CommandRegistry>
    {
        public:
            typedef std::tr1::function<void(const Message *)> handler;
            typedef struct _id { } *id;

            void dispatch(const Message *, bool = false);

            id add_handler(Filter, const handler &);
            void remove_handler(id);

            CommandRegistry();
            ~CommandRegistry();
    };
}

#endif
