/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_JOIN_HH
#define PALUDIS_GUARD_PALUDIS_JOIN_HH 1

#include <paludis/util/stringify.hh>
#include <iterator>
#include <string>

#ifdef PALUDIS_HAVE_CONCEPTS
#  include <concepts>
#endif

/** \file
 * Declarations for the join function.
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
    auto concept IsJoinResult<typename T_>
    {
        requires std::DefaultConstructible<T_>;
        requires std::CopyConstructible<T_>;
        void T_::operator+= (const T_ &);
        void T_::operator+= (const std::string &);
    };

    auto concept IsJoinStringifier<typename F_, typename T_, typename I_>
    {
        T_ operator() (const F_ &, const I_ &);
    };
#endif

    /**
     * Join together the items from i to end using joiner.
     *
     * \ingroup g_strings
     */
    template <typename I_, typename T_>
#ifdef PALUDIS_HAVE_CONCEPTS
        requires
            std::ForwardIterator<I_>,
            IsStringifiable<I_::reference>,
            IsJoinResult<T_>
#endif
    T_ join(I_ i, I_ end, const T_ & joiner)
    {
        T_ result;
        if (i != end)
            while (true)
            {
                result += stringify(*i);
                if (++i == end)
                    break;
                result += joiner;
            }
        return result;
    }

    /**
     * Join together the items from i to end using joiner, using
     * a function other than stringify.
     *
     * \ingroup g_strings
     */
    template <typename I_, typename T_, typename F_>
#ifdef PALUDIS_HAVE_CONCEPTS
        requires
            std::ForwardIterator<I_>,
            IsJoinResult<T_>,
            IsJoinStringifier<F_, T_, I_::reference>
#endif
    T_ join(I_ i, I_ end, const T_ & joiner, const F_ & f)
    {
        T_ result;
        if (i != end)
            while (true)
            {
                result += (f)(*i);
                if (++i == end)
                    break;
                result += joiner;
            }
        return result;
    }

    /**
     * Convenience alternative join allowing a char * to be used for a
     * string.
     *
     * \ingroup g_strings
     */
    template <typename I_>
#ifdef PALUDIS_HAVE_CONCEPTS
        requires
            std::ForwardIterator<I_>,
            IsStringifiable<I_::reference>
#endif
    std::string join(I_ begin, const I_ end, const char * const t)
    {
        return join(begin, end, std::string(t));
    }

    /**
     * Convenience alternative join allowing a char * to be used for a
     * string, using a function other than stringify.
     *
     * \ingroup g_strings
     */
    template <typename I_, typename F_>
#ifdef PALUDIS_HAVE_CONCEPTS
        requires
            std::ForwardIterator<I_>,
            IsJoinStringifier<F_, std::string, I_::reference>
#endif
    std::string join(I_ begin, const I_ end, const char * const t, const F_ & f)
    {
        return join(begin, end, std::string(t), f);
    }
}

#endif
