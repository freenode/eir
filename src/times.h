#ifndef times_h
#define times_h

#include <string>
#include <ctime>
#include <cstdlib>

#include "exceptions.h"

namespace eir
{
    /* Parses "<number><unit>" into a number of seconds.
     * h == hour, d == day, m == month (four weeks, for simplicity)
     */
    time_t parse_time(std::string ts)
    {
        std::string::iterator b = ts.begin(), e = ts.end();
        int val = 0, mult;

        if (*b == '~')
            ++b;
        switch (*--e)
        {
            case 'h':
                mult = 3600;
                e = ts.erase(e);
                break;
            case 'd':
                mult = 3600 * 24;
                e = ts.erase(e);
                break;
            case 'm':
                mult = 3600 * 24 * 28;
                e = ts.erase(e);
                break;
            default:
                mult = 1;
                break;
        }

        val = atoi(std::string(b, ++e).c_str());

        return val * mult;
    }

}

#endif
