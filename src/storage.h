#ifndef storage_h
#define storage_h

#include "value.h"

namespace eir
{
    class StorageBackend
    {
        public:
            virtual void Save(const eir::Value &, std::string) = 0;
            virtual eir::Value Load(std::string) = 0;

            virtual ~StorageBackend() { }
    };

    class StorageManager : public paludis::PrivateImplementationPattern<StorageManager>,
                           public paludis::InstantiationPolicy<StorageManager, paludis::instantiation_method::SingletonTag>
    {
        public:
            void Save(const eir::Value &, std::string);
            eir::Value Load(std::string);

            typedef unsigned int BackendId;
            BackendId register_backend(std::string, StorageBackend *);
            void unregister_backend(BackendId);

            std::string default_backend();
            void default_backend(std::string);

            StorageManager();
            ~StorageManager();
    };
}

#endif
