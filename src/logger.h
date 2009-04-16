#ifndef logger_h
#define logger_h

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>

#include <string>
#include <tr1/functional>
#include <tr1/memory>

namespace eir
{
    class Client;

    class LogDestination
    {
        public:
            virtual void Log(Client *, std::string) = 0;
            virtual ~LogDestination() { }
    };

    class LogBackend
    {
        public:
            virtual LogDestination* create_destination(std::string) = 0;
            virtual ~LogBackend() { }
    };


    class Logger : public paludis::PrivateImplementationPattern<Logger>,
                   public paludis::InstantiationPolicy<Logger, paludis::instantiation_method::SingletonTag>
    {
        public:
            enum
            {
                Debug   = 0x01,
                Command = 0x02,
                Info    = 0x04,
                Privs   = 0x08,
                Warning = 0x10,
                Raw     = 0x20,
                Admin   = 0x40,

                All     = 0xffffffff
            };
            typedef unsigned int Type;

            void Log(Client *, Type, std::string);
            void Log(std::tr1::shared_ptr<Client>, Type, std::string);

            typedef unsigned int BackendId;
            BackendId register_backend(std::string, LogBackend *);
            void unregister_backend(BackendId);

            typedef unsigned int DestinationId;
            DestinationId add_destination(std::string type, std::string arg, Type types);
            void remove_destination(DestinationId);

            Logger();
            ~Logger();
    };
}

#endif
