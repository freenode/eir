#ifndef string_util_h
#define string_util_h

#include <string>
#include <algorithm>
#include <iterator>
#include <cctype>

namespace eir
{
    inline std::string lowercase(std::string s)
    {
        std::string ret;
        std::transform(s.begin(), s.end(), std::back_inserter(ret),
                       static_cast<int(*)(int)>(std::tolower));
        return ret;
    }

    namespace cistring
    {
        extern unsigned char tolowertab[256];

        inline bool equal(std::string lhs, std::string rhs)
        {
            if (lhs.size() != rhs.size()) return false;

            for (std::string::size_type i=0; i < lhs.size(); i++)
            {
                if (tolowertab[(unsigned char)lhs[i]] != tolowertab[(unsigned char)rhs[i]]) return false;
            }
            return true;
        }

        inline bool less(std::string lhs, std::string rhs)
        {
            for (std::string::size_type i=0; ; i++)
            {
                if (i == lhs.size()) return true;
                if (i == rhs.size()) return false;
                if (tolowertab[(unsigned char)lhs[i]] < tolowertab[(unsigned char)rhs[i]]) return true;
                if (tolowertab[(unsigned char)lhs[i]] > tolowertab[(unsigned char)rhs[i]]) return false;
            }
            return false;
        }

        inline unsigned long hash(std::string arg)
        {
            unsigned long ret = 5381;
            for (std::string::size_type i=0; i < arg.size(); ++i)
                ret = ret * 33 ^ tolowertab[(unsigned char)arg[i]];
            return ret;
        }

        struct is_equal
        {
            bool operator() (std::string l, std::string r) const { return equal(l, r); }
        };
        struct is_less
        {
            bool operator() (std::string l, std::string r) const { return less(l, r); }
        };
        struct hasher
        {
            bool operator() (std::string s) const { return hash(s); }
        };
    }
}

#endif
