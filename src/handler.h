#ifndef handler_h
#define handler_h

#include "command.h"
#include <functional>


namespace eir
{
    template <class T_>
    struct CommandHandlerBase
    {
        template<class F_>
        CommandRegistry::id add_handler(std::string s, F_ f)
        {
            return eir::CommandRegistry::get_instance()->add_handler(s,
                    std::tr1::bind(f, static_cast<T_*>(this), std::tr1::placeholders::_1));
        }

        void remove_handler(CommandRegistry::id i)
        {
            eir::CommandRegistry::get_instance()->remove_handler(i);
        }
    };
}

#endif
