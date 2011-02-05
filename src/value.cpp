#include "value.h"

#include <vector>
#include <map>
#include <memory>

#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>

#include <paludis/util/destringify.hh>

using namespace eir;
using namespace paludis;

template class paludis::WrappedForwardIterator<KeyValueArray::KeyValueArrayIteratorTag, std::pair<const std::string, Value> >;
template class paludis::WrappedForwardIterator<KeyValueArray::KeyValueArrayIteratorTag, const std::pair<const std::string, Value> >;
template class paludis::WrappedForwardIterator<ValueArray::ValueArrayIteratorTag, Value>;
template class paludis::WrappedForwardIterator<ValueArray::ValueArrayIteratorTag, const Value>;

namespace paludis
{
    template <>
    struct Implementation<Value>
    {
        // This shouldn't really be mutable, but it lets us initialise an empty type
        // to a meaningful one in what are still semantically const operations.
        mutable Value::ValueType _type;
        int _intval;
        std::string _stringval;
        std::shared_ptr<ValueArray> _array;
        std::shared_ptr<KeyValueArray> _kv_array;

        Implementation(Value::ValueType t)
            : _type(t)
        {  }

        void clear()
        {
            _intval = 0;
            _stringval = "";
            _array.reset();
            _kv_array.reset();
        }

        void NeedType(Value::ValueType t) const
        {
            if (_type == Value::empty)
            {
                switch (t)
                {
                    case Value::empty:
                    case Value::integer:
                    case Value::string:
                        // These three we can switch to without needing to initialise.
                        _type = t;
                        return;
                    case Value::array:
                    case Value::kvarray:
                        // These two we can't.
                        throw TypeMismatchException(t, _type);
                }
            }
            else if (_type != t)
            {
                throw TypeMismatchException(t, _type);
            }
        }

        void NeedType(Value::ValueType t)
        {
            if (_type == Value::empty)
            {
                _type = t;
            }
            else if (_type != t)
            {
                throw TypeMismatchException(t, _type);
            }
            switch (t)
            {
                case Value::empty:
                case Value::integer:
                case Value::string:
                    return;
                case Value::array:
                    if (!_array) _array.reset(new ValueArray);
                    return;
                case Value::kvarray:
                    if (!_kv_array) _kv_array.reset(new KeyValueArray);
                    return;
            }
        }
    };

    template <>
    struct Implementation<ValueArray>
    {
        std::vector<Value> _array;
    };

    template <>
    struct Implementation<KeyValueArray>
    {
        std::map<std::string, Value> _map;
    };
}

Value::Value()
    : PrivateImplementationPattern<Value>(new Implementation<Value>(empty))
{
}

Value::Value(Value::ValueType t)
    : PrivateImplementationPattern<Value>(new Implementation<Value>(t))
{
    if (t == array)
        _imp->_array.reset(new ValueArray);
    if (t == kvarray)
        _imp->_kv_array.reset(new KeyValueArray);
}

Value::Value(int i)
    : PrivateImplementationPattern<Value>(new Implementation<Value>(integer))
{
    _imp->_intval = i;
}

Value::Value(const char *s)
    : PrivateImplementationPattern<Value>(new Implementation<Value>(string))
{
    _imp->_stringval = s;
}

Value::Value(std::string s)
    : PrivateImplementationPattern<Value>(new Implementation<Value>(string))
{
    _imp->_stringval = s;
}

Value::Value(const Value& rhs)
    : PrivateImplementationPattern<Value>(new Implementation<Value>(rhs.Type()))
{
    switch(Type())
    {
        case empty:
            break;
        case integer:
            _imp->_intval = rhs.Int();
            break;
        case string:
            _imp->_stringval = (std::string)rhs;
            break;
        case array:
            _imp->_array = rhs._imp->_array;
            break;
        case kvarray:
            _imp->_kv_array = rhs._imp->_kv_array;
            break;
    }
}

const Value& Value::operator= (const Value& rhs)
{
    if (Type() != rhs.Type())
    {
        _imp->clear();
        _imp->_type = rhs.Type();
    }

    switch(Type())
    {
        case empty:
            break;
        case integer:
            _imp->_intval = rhs.Int();
            break;
        case string:
            _imp->_stringval = (std::string)rhs;
            break;
        case array:
            _imp->_array = rhs._imp->_array;
            break;
        case kvarray:
            _imp->_kv_array = rhs._imp->_kv_array;
            break;
    }

    return *this;
}


Value::~Value()
{
}

Value::ValueType Value::Type() const
{
    return _imp->_type;
}

const Value& Value::operator=(int i)
{
    if (_imp->_type != integer)
        _imp->clear();

    _imp->_type = integer;
    _imp->_intval = i;
    return *this;
}

const Value& Value::operator=(const std::string& s)
{
    if (_imp->_type != string)
        _imp->clear();

    _imp->_type = string;
    _imp->_stringval = s;
    return *this;
}

/*
Value::operator int() const
{
    if (_imp->_type != Int)
        throw TypeMismatchException(Int, _imp->_type);
    return _imp->_intval;
}
*/

Value::operator std::string() const
{
    return String();
}

Value::operator bool() const
{
    switch (Type())
    {
        case empty:
            return false;
        case integer:
            return _imp->_intval != 0;
        case string:
            return !_imp->_stringval.empty();
        case array:
            return !_imp->_array->empty();
        case kvarray:
            return !_imp->_kv_array->empty();
    }
    throw InternalError("Don't know what type of value I am.");
}

int Value::Int() const
{
    switch(Type())
    {
        case integer:
            return _imp->_intval;
        case string:
            try
            {
                return paludis::destringify<int>(_imp->_stringval);
            }
            catch (paludis::DestringifyError)
            {
                throw TypeMismatchException(integer, Type());
            }
        default:
            throw TypeMismatchException(integer, Type());
    }
}

std::string Value::String() const
{
    switch(Type())
    {
        case Value::empty:
            return "<null>";
        case Value::integer:
            return stringify(_imp->_intval);
        case Value::string:
            return _imp->_stringval;
        case Value::array:
            return "<Array>";
        case Value::kvarray:
            return "<KVArray>";
    }
    throw InternalError("I don't know what type I am. Help.");
}

ValueArray& Value::Array()
{
    _imp->NeedType(array);

    return *_imp->_array;
}

const ValueArray& Value::Array() const
{
    _imp->NeedType(array);

    return *_imp->_array;
}

KeyValueArray& Value::KV()
{
    _imp->NeedType(kvarray);

    return *_imp->_kv_array;
}

const KeyValueArray& Value::KV() const
{
    _imp->NeedType(kvarray);

    return *_imp->_kv_array;
}

Value& Value::operator[](int i)
{
    if (Type() == kvarray)
        return KV()[Value(i)];

    if (Type() == array)
        return Array()[i];

    throw TypeMismatchException(array, Type());
}


Value& Value::operator[](const std::string& s)
{
    if (Type() == empty)
        _imp->NeedType(kvarray);

    if (Type() == kvarray)
        return KV()[s];

    throw TypeMismatchException(array, Type());
}

void Value::push_back(Value v)
{
    _imp->NeedType(array);

    _imp->_array->push_back(v);
}

ValueArray::iterator Value::erase(ValueArray::iterator it)
{
    _imp->NeedType(array);

    return _imp->_array->erase(it);
}

void Value::clear()
{
    switch (Type())
    {
        case array:
            _imp->_array->clear();
            break;
        case kvarray:
            _imp->_kv_array->clear();
            break;
        default:
            break;
    }
}

ValueArray::iterator Value::begin()
{
    _imp->NeedType(array);

    return Array().begin();
}

ValueArray::iterator Value::end()
{
    _imp->NeedType(array);

    return Array().end();
}


///////////////////////////////////////////////////////////////////////
// ValueArray stuff


ValueArray::const_iterator ValueArray::begin() const
{
    return _imp->_array.begin();
}

ValueArray::iterator ValueArray::begin()
{
    return _imp->_array.begin();
}

ValueArray::const_iterator ValueArray::end() const
{
    return _imp->_array.end();
}

ValueArray::iterator ValueArray::end()
{
    return _imp->_array.end();
}

size_t ValueArray::size() const
{
    return _imp->_array.size();
}

void ValueArray::resize(size_t newsize)
{
    _imp->_array.resize(newsize);
}

void ValueArray::push_back(Value v)
{
    _imp->_array.push_back(v);
}

void ValueArray::pop_back()
{
    _imp->_array.pop_back();
}

Value& ValueArray::back()
{
    return _imp->_array.back();
}

const Value& ValueArray::back() const
{
    return _imp->_array.back();
}

void ValueArray::insert(ValueArray::iterator it, const Value& v)
{
    _imp->_array.insert(it.underlying_iterator<std::vector<Value>::iterator>(), v);
}

void ValueArray::insert(size_t idx, const Value& v)
{
    _imp->_array.insert(_imp->_array.begin() + idx, v);
}

ValueArray::iterator ValueArray::erase(ValueArray::iterator it)
{
    return _imp->_array.erase(it.underlying_iterator<std::vector<Value>::iterator>());
}

void ValueArray::erase(size_t idx)
{
    _imp->_array.erase(_imp->_array.begin() + idx);
}

void ValueArray::clear()
{
    _imp->_array.clear();
}

bool ValueArray::empty() const
{
    return _imp->_array.empty();
}

Value& ValueArray::operator[](size_t i)
{
    return _imp->_array.at(i);
}

const Value& ValueArray::operator[](size_t i) const
{
    return _imp->_array.at(i);
}

ValueArray::ValueArray()
    : PrivateImplementationPattern<ValueArray>(new Implementation<ValueArray>)
{
}

ValueArray::~ValueArray()
{
}

// KeyValueArray stuff

KeyValueArray::KeyValueArray()
    : PrivateImplementationPattern<KeyValueArray>(new Implementation<KeyValueArray>)
{
}

KeyValueArray::~KeyValueArray()
{
}

KeyValueArray::iterator KeyValueArray::begin()
{
    return _imp->_map.begin();
}

KeyValueArray::const_iterator KeyValueArray::begin() const
{
    return _imp->_map.begin();
}

KeyValueArray::iterator KeyValueArray::end()
{
    return _imp->_map.end();
}

KeyValueArray::const_iterator KeyValueArray::end() const
{
    return _imp->_map.end();
}

KeyValueArray::iterator KeyValueArray::find(std::string s)
{
    return _imp->_map.find(s);
}

KeyValueArray::const_iterator KeyValueArray::find(std::string s) const
{
    return _imp->_map.find(s);
}

size_t KeyValueArray::size() const
{
    return _imp->_map.size();
}

bool KeyValueArray::insert(std::string s, Value v)
{
    return _imp->_map.insert(std::make_pair(s, v)).second;
}

bool KeyValueArray::erase(std::string s)
{
    return _imp->_map.erase(s) > 0;
}

KeyValueArray::iterator KeyValueArray::erase(KeyValueArray::iterator it)
{
    return _imp->_map.erase(it.underlying_iterator<std::map<std::string,Value>::iterator>());
}

void KeyValueArray::clear()
{
    _imp->_map.clear();
}

bool KeyValueArray::empty() const
{
    return _imp->_map.empty();
}

Value& KeyValueArray::operator[](std::string s)
{
    return _imp->_map[s];
}

bool Value::operator!() const
{
    switch (Type())
    {
        case empty:
            return true;
        case integer:
            return _imp->_intval == 0;
        case string:
            return _imp->_stringval.empty();
        case array:
            return _imp->_array->empty();
        case kvarray:
            return _imp->_kv_array->empty();
    }
    throw InternalError("I don't know what type I am. Help.");
}

std::ostream& eir::operator<<(std::ostream & os, const Value & v)
{
    switch(v.Type())
    {
        case Value::empty:
            os << "<null>";
            break;
        case Value::integer:
            os << v._imp->_intval;
            break;
        case Value::string:
            os << v._imp->_stringval;
            break;
        case Value::array:
            os << "<Array>";
            break;
        case Value::kvarray:
            os << "<KVArray>";
            break;
    }
    return os;
}

namespace
{
    const std::string & ValueTypeToString(Value::ValueType t)
    {
        static std::string IntString("integer"), StringString("string"),
                           ArrayString("array"), KVArrayString("kvarray"),
                           EmptyString("empty"), Unknown("<unknown>");
        switch (t)
        {
            case Value::empty:   return EmptyString;
            case Value::integer: return IntString;
            case Value::string:  return StringString;
            case Value::array:   return ArrayString;
            case Value::kvarray: return KVArrayString;
        }
        return Unknown;
    }
}

TypeMismatchException::TypeMismatchException(Value::ValueType t1, Value::ValueType t2)
    : Exception("Type mismatch: expected " + ValueTypeToString(t1) + ", found " + ValueTypeToString(t2))
{
}
