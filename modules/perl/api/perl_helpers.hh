#ifndef perl_helpers_hh
#define perl_helpers_hh

#include <functional>

namespace eir
{
    namespace perl
    {
        class PerlHolder
        {
            public:
                virtual ~PerlHolder() = 0;
        };

        class PerlCommandHolder : public PerlHolder
        {
            CommandRegistry::id _id;
            SV *_command;
            PerlInterpreter *_perl;

            public:
                PerlCommandHolder(pTHX_ CommandRegistry::id i, SV *cmd)
                    : _id(i), _command(cmd), _perl(aTHX)
                {
                    SvREFCNT_inc(cmd);
                }

                ~PerlCommandHolder()
                {
                    PerlInterpreter *my_perl = _perl;
                    CommandRegistry::get_instance()->remove_handler(_id);
                    SvREFCNT_dec(_command);
                }
        };

        class PerlEventHolder : public PerlHolder
        {
            EventManager::id _id;
            SV *_handler;
            PerlInterpreter *_perl;

            public:
                PerlEventHolder(pTHX_ EventManager::id i, SV *h)
                    : _id(i), _handler(h), _perl(aTHX)
                {
                    SvREFCNT_inc(h);
                }
                ~PerlEventHolder()
                {
                    PerlInterpreter *my_perl = _perl;
                    EventManager::get_instance()->remove_event(_id);
                    SvREFCNT_dec(_handler);
                }
        };
    }
}

#endif
