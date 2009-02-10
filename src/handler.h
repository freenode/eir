#ifndef handler_h
#define handler_h

#include "command.h"
#include <functional>


namespace eir
{
    template <class T_>
    struct CommandHandlerBase
    {
        template <class F_>
        CommandRegistry::id add_handler(std::string s, F_ f)
        {
            return eir::CommandRegistry::get_instance()->add_handler(s,
                    std::tr1::bind(f, static_cast<T_*>(this), std::tr1::placeholders::_1));
        }

        template <class F_>
        CommandRegistry::id add_handler(std::string s, unsigned int t, F_ f)
        {
            return eir::CommandRegistry::get_instance()->add_handler(s, t,
                    std::tr1::bind(f, static_cast<T_*>(this), std::tr1::placeholders::_1));
        }
    };
}

#endif
