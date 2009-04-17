/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Stephen Bennett
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

#ifndef PALUDIS_GUARD_PALUDIS_DESTRINGIFY_HH
#define PALUDIS_GUARD_PALUDIS_DESTRINGIFY_HH 1

#include <paludis/util/exception.hh>
#include <sstream>
#include <string>

/** \file
 * Destringify functions.
 *
 * \ingroup g_strings
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Default exception type thrown by destringify when it fails.
     *
     * \ingroup g_strings
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DestringifyError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            DestringifyError(const std::string & str) throw ();

            ///\}
    };

    /**
     * For internal use by destringify.
     *
     * \ingroup g_strings
     */
    namespace destringify_internals
    {
        /**
         * Basic destringifier.
         *
         * \ingroup g_strings
         */
        template <typename Type_, typename Exception_>
        struct Destringifier
        {
            /**
             * Do the destringification.
             */
            static Type_ do_destringify(const std::string & s)
            {
                std::istringstream ss(s);
                Type_ t;
                ss >> t;
                if (! ss.eof() || ss.fail())
                    throw Exception_(s);
                return t;
            }
        };

        /**
         * Specialised destringify for std::string.
         *
         * \ingroup g_strings
         */
        template <typename Exception_>
        struct Destringifier<std::string, Exception_>
        {
            /**
             * Do the destringification.
             */
            static std::string do_destringify(const std::string & s)
            {
                return s;
            }
        };

        /**
         * Specialised destringify for bool.
         *
         * \ingroup g_strings
         */
        template <typename Exception_>
        struct Destringifier<bool, Exception_>
        {
            /**
             * Do the destringification.
             */
            static bool do_destringify(const std::string & s)
            {
                /* Don't use boolalpha on std::istringstream here, since it's way too difficult
                 * to test for errors and eof. See gcc bug 37958. */
                if (s == "true")
                    return true;
                else if (s == "false")
                    return false;
                else
                {
                    std::istringstream ss(s);
                    int i;
                    ss >> i;
                    if (ss.eof() && ! ss.bad())
                        return i > 0;
                    else
                        throw Exception_(s);
                }
            }
        };

        /**
         * Specialised destringify for char.
         *
         * \ingroup g_strings
         */
        template <typename Exception_>
        struct Destringifier<char, Exception_>
        {
            /**
             * Do the destringification.
             */
            static char do_destringify(const std::string & s)
            {
                if (s.length() == 1)
                    return s[0];
                else
                    throw Exception_(s);
            }
        };
    }

#ifdef PALUDIS_HAVE_CONCEPTS
    /**
     * A type is destringifiable if it can be default and copy constructed and read from an ostream.
     *
     * \ingroup g_strings
     * \since 0.26
     */
    auto concept IsDestringifiable<typename T_>
    {
        requires std::DefaultConstructible<T_>;
        requires std::CopyConstructible<T_>;
        std::ostream & operator<< (std::ostream &, const T_ &);
    };

    /**
     * A type is suitable for destringify exceptions if it can be thrown with a single string parameter.
     *
     * \ingroup g_strings
     * \since 0.26
     */
    auto concept IsDestringifyException<typename T_>
    {
        requires std::CopyConstructible<T_>;
        T_::T_(const std::string &);
    };
#endif

    /**
     * Extract a value of some type from a string.
     *
     * \ingroup g_strings
     */
    template <typename Type_, typename Exception_>
#ifdef PALUDIS_HAVE_CONCEPTS
            requires
                IsDestringifiable<Type_>,
                IsDestringifyException<Exception_>
#endif
    Type_ destringify(const std::string & s)
    {
        if (s == "")
            throw Exception_("");

        return destringify_internals::Destringifier<Type_, Exception_>::do_destringify(s);
    }

    template <typename Type_>
#ifdef PALUDIS_HAVE_CONCEPTS
            requires
                IsDestringifiable<Type_>
#endif
    Type_ destringify(const std::string & s)
    {
        return destringify<Type_, DestringifyError>(s);
    }
}

#endif
