#ifndef singleton_h
#define singleton_h

namespace eir {
    namespace util {

        template <class T>
        class Singleton
        {
            protected:
                Singleton() {}

            public:
                static T* get_instance() {
                    static T _instance;
                    return &_instance;
                }
        };

    }
}

#endif
