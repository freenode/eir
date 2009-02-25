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
}

#endif
