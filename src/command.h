#ifndef command_h
#define command_h

#include <map>
#include <list>
#include <tr1/functional>
#include "message.h"
#include "singleton.h"

namespace eir
{
    class CommandRegistry : public util::Singleton<CommandRegistry>
    {
        public:
            typedef std::tr1::function<void(const Message *)> handler;
            typedef struct _id { } *id;

            void dispatch(const Message *);

            id add_handler(std::string, const handler &);
            void remove_handler(id);

        private:
            struct IrcStringCmp {
                bool operator() (std::string, std::string);
            };
            typedef std::map<id, handler> HandlerList;
            typedef std::map<std::string, HandlerList, IrcStringCmp> HandlerMap;
            HandlerMap _handlers;
    };
}

#endif
