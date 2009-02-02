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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MEMBER_ITERATOR_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MEMBER_ITERATOR_IMPL_HH 1

#include <paludis/util/member_iterator.hh>

namespace paludis
{
    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    MemberIterator<Value_, Iterator_, member_>::MemberIterator()
    {
    }

    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    MemberIterator<Value_, Iterator_, member_>::MemberIterator(const MemberIterator & other) :
        _iter(other._iter)
    {
    }

    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    MemberIterator<Value_, Iterator_, member_>::MemberIterator(const Iterator_ & iter) :
        _iter(iter)
    {
    }

    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    MemberIterator<Value_, Iterator_, member_> &
    MemberIterator<Value_, Iterator_, member_>::operator= (const MemberIterator & other)
    {
        _iter = other._iter;
        return *this;
    }

    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    MemberIterator<Value_, Iterator_, member_> &
    MemberIterator<Value_, Iterator_, member_>::operator++ ()
    {
        ++_iter;
        return *this;
    }

    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    MemberIterator<Value_, Iterator_, member_>
    MemberIterator<Value_, Iterator_, member_>::operator++ (int)
    {
        MemberIterator result(*this);
        ++_iter;
        return result;
    }

    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    typename MemberIterator<Value_, Iterator_, member_>::pointer
    MemberIterator<Value_, Iterator_, member_>::operator-> () const
    {
        return &((*_iter).*member_);
    }

    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    typename MemberIterator<Value_, Iterator_, member_>::reference
    MemberIterator<Value_, Iterator_, member_>::operator* () const
    {
        return (*_iter).*member_;
    }

    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    bool
    MemberIterator<Value_, Iterator_, member_>::operator== (const MemberIterator & other) const
    {
        return _iter == other._iter;
    }

    template <typename Iterator_>
    typename FirstIteratorTypes<Iterator_>::Type
    first_iterator(Iterator_ i)
    {
        return typename FirstIteratorTypes<Iterator_>::Type(i);
    }

    template <typename Iterator_>
    typename SecondIteratorTypes<Iterator_>::Type
    second_iterator(Iterator_ i)
    {
        return typename SecondIteratorTypes<Iterator_>::Type(i);
    }
}

#endif
