/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PALUDIS_GUARD_PALUDIS_STRINGIFY_HH
#define PALUDIS_GUARD_PALUDIS_STRINGIFY_HH 1

#include <paludis/util/attributes.hh>
#include <tr1/memory>
#include <sstream>
#include <string>

#ifdef PALUDIS_HAVE_CONCEPTS
#  include <concepts>
#endif

/** \file
 * Stringify functions.
 *
 * \ingroup g_strings
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
#ifdef PALUDIS_HAVE_CONCEPTS
    auto concept IsStreamStringifiable<typename T_>
    {
        requires ! std::Dereferenceable<T_>;
        std::ostream & operator<< (std::ostream &, const T_ &);
    };

    auto concept IsStringifiable<typename T_>
    {
        std::string stringify(const T_ &);
    };
#endif

#ifndef PALUDIS_HAVE_CONCEPTS
    /**
     * For use by stringify.
     *
     * \ingroup g_strings
     */
    namespace stringify_internals
    {
        /**
         * Check that T_ is a sane type to be stringified.
         *
         * \ingroup g_strings
         */
        template <typename T_>
        struct CheckType
        {
            /// Yes, we are a sane type.
            enum { value = 0 } Value;
        };

        /**
         * Check that T_ is a sane type to be stringified, which it isn't
         * if it's a pointer unless it's a char * pointer.
         *
         * \ingroup g_strings
         */
        template <typename T_>
        struct CheckType<T_ *>
        {
        };

        /**
         * Check that T_ is a sane type to be stringified, which it isn't
         * if it's a CountedPtr.
         *
         * \ingroup g_strings
         */
        template <typename T_>
        struct CheckType<std::tr1::shared_ptr<T_> >
        {
        };

        /**
         * Check that T_ is a sane type to be stringified, which it isn't
         * if it's a pointer unless it's a char * pointer.
         *
         * \ingroup g_strings
         */
        template <>
        struct CheckType<char *>
        {
            /// Yes, we are a sane type.
            enum { value = 0 } Value;
        };
    }
#endif

    template <typename T_> inline std::string stringify(const T_ & item);

    namespace stringify_internals
    {
        /**
         * Internal function to convert item to a string, to make
         * function pointers work more sensibly.  May be overloaded,
         * but should not be called directly.
         *
         * \ingroup g_strings
         */
        template <typename T_>
#ifdef PALUDIS_HAVE_CONCEPTS
            requires IsStreamStringifiable<T_>
#endif
        std::string
        real_stringify(const T_ & item)
        {
#ifndef PALUDIS_HAVE_CONCEPTS
            /* check that we're not trying to stringify a pointer or somesuch */
            int check_for_stringifying_silly_things
                PALUDIS_ATTRIBUTE((unused)) = CheckType<T_>::value;
#endif

            std::ostringstream s;
            s << item;
            return s.str();
        }

        inline std::string
        real_stringify(const std::string & item)
        {
            return item;
        }

        inline std::string
        real_stringify(const char & item)
        {
            return std::string(1, item);
        }

        inline std::string
        real_stringify(const unsigned char & item)
        {
            return std::string(1, item);
        }

        inline std::string
        real_stringify(const bool & item)
        {
            return item ? "true" : "false";
        }

        inline std::string
        real_stringify(const char * const item)
        {
            return std::string(item);
        }
    }

    /**
     * Convert item to a string.  To customise for new types, overload
     * stringify_internals::real_stringify, not this function.
     *
     * \ingroup g_strings
     */
    template <typename T_>
    inline std::string
    stringify(const T_ & item)
    {
        return stringify_internals::real_stringify(item);
    }
}

#endif
