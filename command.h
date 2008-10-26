#ifndef command_h
#define command_h

#include <map>
#include <list>
#include <tr1/functional>
#include "message.h"

namespace eir
{
    class Dispatcher
    {
        public:
            typedef std::tr1::function<void(const Message&)> handler;

            void dispatch(const Message &);

            void add_handler(std::string, const handler &);

        private:
            struct IrcStringCmp {
                bool operator() (std::string, std::string);
            };
            typedef std::list<handler> HandlerList;
            typedef std::map<std::string, HandlerList, IrcStringCmp> HandlerMap;
            HandlerMap _handlers;
    };
}

#endif
