/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
 * Copyright (c) 2007 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MEMBER_ITERATOR_FWD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MEMBER_ITERATOR_FWD_HH 1

#include <iterator>
#include <tr1/type_traits>

namespace paludis
{
    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    class MemberIterator;

    template <typename Iterator_>
    struct FirstIteratorTypes;

    template <typename Iterator_>
    typename FirstIteratorTypes<Iterator_>::Type
    first_iterator(Iterator_);

    template <typename Iterator_>
    struct SecondIteratorTypes;

    template <typename Iterator_>
    typename SecondIteratorTypes<Iterator_>::Type
    second_iterator(Iterator_);
}

#endif
