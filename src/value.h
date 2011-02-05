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

            typedef Value& reference;
            typedef const Value& const_reference;
            typedef Value value_type;

            iterator begin();
            iterator end();
            const_iterator begin() const;
            const_iterator end() const;

            size_t size() const;
            bool empty() const;
            void resize(size_t newsize);

            void push_back(Value);
            void pop_back();
            Value& back();
            const Value& back() const;

            void insert(iterator, const Value&);
            void insert(size_t, const Value&);
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
            typedef std::pair<const std::string, Value> value_type;
            typedef paludis::WrappedForwardIterator<KeyValueArrayIteratorTag, value_type> iterator;
            typedef paludis::WrappedForwardIterator<KeyValueArrayIteratorTag, const value_type> const_iterator;

            typedef Value& reference;
            typedef const Value& const_reference;

            iterator begin();
            iterator end();
            const_iterator begin() const;
            const_iterator end() const;

            iterator find(std::string);
            const_iterator find(std::string) const;

            size_t size() const;
            bool empty() const;

            bool insert(std::string, Value);

            bool erase(std::string);
            void erase(iterator);
            void clear();

            Value& operator[](std::string);

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
            explicit operator bool() const;

            ValueArray::iterator begin();
            ValueArray::iterator end();

            int Int() const;
            std::string String() const;
            ValueArray& Array();
            const ValueArray& Array() const;
            KeyValueArray& KV();
            const KeyValueArray& KV() const;

            Value& operator[](const std::string&);
            Value& operator[](int);

            void push_back(Value v);
            void erase(ValueArray::iterator it);

            void clear();

            const char *c_str() const { return String().c_str(); }

            bool operator!() const;

            friend std::ostream& operator<<(std::ostream&, const Value&);
    };

    std::ostream & operator<<(std::ostream&, const Value&);

    inline std::string operator+(std::string lhs, const Value& rhs)
    { return lhs + rhs.String(); }

    inline std::string operator+(const Value& lhs, std::string rhs)
    { return lhs.String() + rhs; }

    inline std::string operator+(const char *lhs, const Value& rhs)
    { return lhs + rhs.String(); }

    inline std::string operator+(const Value& lhs, const char *rhs)
    { return lhs.String() + rhs; }

    inline bool operator==(const Value& lhs, int rhs)
    { return lhs.Type() == Value::integer && lhs.Int() == rhs; }

    inline bool operator==(int lhs, const Value& rhs)
    { return rhs.Type() == Value::integer && lhs == rhs.Int(); }

    inline bool operator==(const Value& lhs, const std::string& rhs)
    { return lhs.Type() == Value::string && lhs.String() == rhs; }

    inline bool operator==(const std::string& lhs, const Value& rhs)
    { return rhs.Type() == Value::string && lhs == rhs.String(); }

    inline bool operator!=(const Value& lhs, int rhs)
    { return lhs.Type() != Value::integer || lhs.Int() != rhs; }

    inline bool operator!=(int lhs, const Value& rhs)
    { return rhs.Type() != Value::integer || lhs != rhs.Int(); }

    inline bool operator!=(const Value& lhs, const std::string& rhs)
    { return lhs.Type() != Value::string || lhs.String() != rhs; }

    inline bool operator!=(const std::string& lhs, const Value& rhs)
    { return rhs.Type() != Value::string || lhs != rhs.String(); }

    class TypeMismatchException : public Exception
    {
        public:
            TypeMismatchException(Value::ValueType expected, Value::ValueType found);
    };
}

#endif
