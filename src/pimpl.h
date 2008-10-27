#ifndef pimpl_h
#define pimpl_h

namespace eir {

    template <class T> struct Implementation;

    namespace util {

        template <class T>
        class Pimpl {
            private:
                Pimpl() {};

            protected:
                explicit Pimpl(Implementation<T> *i) : _imp(i) { }
                Implementation<T> *_imp;
                ~Pimpl() { delete _imp; }
        };

    }
}

#endif
