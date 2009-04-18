#ifndef value_h
#define value_h

#include <string>
#include <iosfwd>

#include "exceptions.h"

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

namespace eir
{
    class Value;

    class ValueArray : public paludis::PrivateImplementationPattern<ValueArray>
    {
        public:
            struct ValueArrayIteratorTag { };
            typedef paludis::WrappedForwardIterator<ValueArrayIteratorTag, Value> iterator;
            typedef paludis::WrappedForwardIterator<ValueArrayIteratorTag, const Value> const_iterator;

            iterator begin();
            iterator end();
            const_iterator begin() const;
            const_iterator end() const;

            size_t size() const;
            bool empty() const;

            void push_back(Value);

            void erase(iterator);
            void erase(size_t);
            void clear();

            Value& operator[](size_t);
            const Value& operator[](size_t) const;

            ValueArray();
            ~ValueArray();
    };

    class KeyValueArray : public paludis::PrivateImplementationPattern<KeyValueArray>
    {
        public:
            struct KeyValueArrayIteratorTag { };
            typedef std::pair<const Value, Value> value_type;
            typedef paludis::WrappedForwardIterator<KeyValueArrayIteratorTag, value_type> iterator;
            typedef paludis::WrappedForwardIterator<KeyValueArrayIteratorTag, const value_type> const_iterator;

            iterator begin();
            iterator end();
            const_iterator begin() const;
            const_iterator end() const;

            iterator find(Value);
            const_iterator find(Value) const;

            size_t size() const;
            bool empty() const;

            bool insert(Value, Value);

            bool erase(Value);
            void erase(iterator);
            void clear();

            Value& operator[](Value);

            KeyValueArray();
            ~KeyValueArray();
    };

    class Value : public paludis::PrivateImplementationPattern<Value>
    {
        public:
            enum ValueType
            {
                empty,
                integer,
                string,
                array,
                kvarray
            };

            Value();
            Value(ValueType);

            Value(int);
            Value(const char *);
            Value(std::string);

            Value(const Value&);
            const Value& operator=(const Value&);

            ~Value();

            ValueType Type() const;

            const Value& operator=(int);
            const Value& operator=(const std::string&);

            // No operator int(), because C++ is immensely silly.
            //operator int() const;
            operator std::string() const;

            ValueArray::iterator begin();
            ValueArray::iterator end();

            int Int() const;
            std::string String() const;
            ValueArray& Array();
            const ValueArray& Array() const;
            KeyValueArray& KV();
            const KeyValueArray& KV() const;

            Value& operator[](const Value&);
            Value& operator[](int);

            void push_back(Value v);

            const char *c_str() const { return String().c_str(); }

            bool operator!() const;

            friend std::ostream& operator<<(std::ostream&, const Value&);
    };

    std::ostream & operator<<(std::ostream&, const Value&);

    class TypeMismatchException : public Exception
    {
        public:
            TypeMismatchException(Value::ValueType expected, Value::ValueType found);
    };
}

#endif
