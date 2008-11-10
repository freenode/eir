/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_OPERATORS_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_OPERATORS_HH 1

#include <paludis/util/attributes.hh>

/**
 * Various classes that simplify implementing operators for classes.
 *
 * \ingroup g_oo
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Classes with a base in this namespace define comparison operators via
     * operator==.
     *
     * \ingroup g_oo
     */
    namespace equality_operators
    {
        /**
         * Classes inheriting this define relational operators via
         * operator==.
         *
         * \ingroup g_oo
         */
        struct HasEqualityOperators
        {
            /**
             * Template voodoo to assert that we're not trying to call operators
             * on something we're not supposed to.
             */
            template <typename T_>
            struct AssertHasEqualityOperators
            {
                typedef T_ Type;
            };
        };

        template <typename T1_>
        inline bool operator!= (
                const T1_ & a,
                const typename T1_::template AssertHasEqualityOperators<T1_>::Type & b) PALUDIS_ATTRIBUTE((warn_unused_result));

        template <typename T1_>
        inline bool operator!= (
                const T1_ & a,
                const typename T1_::template AssertHasEqualityOperators<T1_>::Type & b)
        {
            return ! (a == b);
        }
    }

    /**
     * Classes with a base in this namespace define comparison operators via
     * operator< and operator==.
     *
     * \ingroup g_oo
     */
    namespace relational_operators
    {
        /**
         * Classes inheriting this define relational operators via
         * operator< and operator==.
         *
         * \ingroup g_oo
         */
        struct HasRelationalOperators :
            public equality_operators::HasEqualityOperators
        {
            /**
             * Template voodoo to assert that we're not trying to call operators
             * on something we're not supposed to.
             */
            template <typename T_>
            struct AssertHasRelationalOperators
            {
                typedef T_ Type;
            };
        };

        template <typename T1_>
        inline bool operator> (
                const T1_ & a,
                const typename T1_::template AssertHasRelationalOperators<T1_>::Type & b) PALUDIS_ATTRIBUTE((warn_unused_result));

        template <typename T1_>
        inline bool operator> (
                const T1_ & a,
                const typename T1_::template AssertHasRelationalOperators<T1_>::Type & b)
        {
            return b < a;
        }

        template <typename T1_>
        inline bool operator<= (
                const T1_ & a,
                const typename T1_::template AssertHasRelationalOperators<T1_>::Type & b) PALUDIS_ATTRIBUTE((warn_unused_result));

        template <typename T1_>
        inline bool operator<= (
                const T1_ & a,
                const typename T1_::template AssertHasRelationalOperators<T1_>::Type & b)
        {
            return ! (b < a);
        }

        template <typename T1_>
        inline bool operator>= (
                const T1_ & a,
                const typename T1_::template AssertHasRelationalOperators<T1_>::Type & b) PALUDIS_ATTRIBUTE((warn_unused_result));

        template <typename T1_>
        inline bool operator>= (
                const T1_ & a,
                const typename T1_::template AssertHasRelationalOperators<T1_>::Type & b)
        {
            return ! (a < b);
        }
    }

    /**
     * Classes with a base in this namespace define arithmetic operators via
     * operator+= etc.
     *
     * \ingroup g_oo
     */
    namespace arithmetic_operators
    {
        /**
         * Classes inheriting this define arithmetic operators via
         * via operator+= etc.
         *
         * \ingroup g_oo
         */
        struct HasArithmeticOperators
        {
            /**
             * Template voodoo to assert that we're not trying to call operators
             * on something we're not supposed to.
             */
            template <typename T_>
            struct AssertHasArithmeticOperators
            {
                typedef T_ Type;
            };
        };

        template <typename T1_>
        inline T1_ operator+ (
                const T1_ & a,
                const typename T1_::template AssertHasArithmeticOperators<T1_>::Type & b) PALUDIS_ATTRIBUTE((warn_unused_result));

        template <typename T1_>
        inline T1_ operator+ (
                const T1_ & a,
                const typename T1_::template AssertHasArithmeticOperators<T1_>::Type & b)
        {
            T1_ result(a);
            result += b;
            return result;
        }

        template <typename T1_>
        inline T1_ operator- (
                const T1_ & a,
                const typename T1_::template AssertHasArithmeticOperators<T1_>::Type & b) PALUDIS_ATTRIBUTE((warn_unused_result));

        template <typename T1_>
        inline T1_ operator- (
                const T1_ & a,
                const typename T1_::template AssertHasArithmeticOperators<T1_>::Type & b)
        {
            T1_ result(a);
            result -= b;
            return result;
        }

        template <typename T1_>
        inline T1_ operator* (
                const T1_ & a,
                const typename T1_::template AssertHasArithmeticOperators<T1_>::Type & b) PALUDIS_ATTRIBUTE((warn_unused_result));

        template <typename T1_>
        inline T1_ operator* (
                const T1_ & a,
                const typename T1_::template AssertHasArithmeticOperators<T1_>::Type & b)
        {
            T1_ result(a);
            result *= b;
            return result;
        }

        template <typename T1_>
        inline T1_ operator/ (
                const T1_ & a,
                const typename T1_::template AssertHasArithmeticOperators<T1_>::Type & b) PALUDIS_ATTRIBUTE((warn_unused_result));

        template <typename T1_>
        inline T1_ operator/ (
                const T1_ & a,
                const typename T1_::template AssertHasArithmeticOperators<T1_>::Type & b)
        {
            T1_ result(a);
            result /= b;
            return result;
        }
    }
}

#endif
