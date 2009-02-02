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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MEMBER_ITERATOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MEMBER_ITERATOR_HH 1

#include <paludis/util/member_iterator-fwd.hh>
#include <paludis/util/operators.hh>
#include <tr1/type_traits>

#ifdef PALUDIS_HAVE_CONCEPTS
#  include <concepts>
#endif


namespace paludis
{
    template <typename Iterator_>
    struct FirstIteratorTypes
    {
        typedef MemberIterator<
            typename std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::first_type,
            Iterator_,
            &std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::first>
                Type;
    };

    template <typename Iterator_>
    struct SecondIteratorTypes
    {
        typedef MemberIterator<
            typename std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::second_type,
            Iterator_,
            &std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::second>
                Type;
    };

    /**
     * A MemberIterator is a wrapper around a forward iterator to a struct,
     * selecting one particular member of that struct for the value.
     *
     * \ingroup g_iterator
     * \since 0.26
     */
    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    class MemberIterator :
        public equality_operators::HasEqualityOperators
    {
        private:
            Iterator_ _iter;

        public:
            ///\name Basic operations
            ///\{

            MemberIterator();
            MemberIterator(const MemberIterator &);
            MemberIterator(const Iterator_ &);

            MemberIterator & operator= (const MemberIterator &);

            ///\}

            ///\name Standard library typedefs
            ///\{

            typedef typename std::tr1::remove_const<typename std::tr1::remove_reference<Value_>::type>::type value_type;
            typedef const typename std::tr1::remove_reference<Value_>::type & reference;
            typedef const typename std::tr1::remove_reference<Value_>::type * pointer;
            typedef std::ptrdiff_t difference_type;
            typedef std::forward_iterator_tag iterator_category;

            ///\}

            ///\name Increment
            ///\{

            MemberIterator & operator++ ();
            MemberIterator operator++ (int);

            ///\}

            ///\name Dereference
            ///\{

            pointer operator-> () const;
            reference operator* () const;

            ///\}

            ///\name Equality
            ///\{

            bool operator== (const MemberIterator &) const;

            ///\}
    };
}

#ifdef PALUDIS_HAVE_CONCEPTS
namespace std
{
    template <typename Value_, typename Iterator_,
             Value_ std::tr1::remove_reference<typename std::iterator_traits<Iterator_>::value_type>::type::* member_>
    concept_map ForwardIterator<paludis::MemberIterator<Value_, Iterator_, member_> >
    {
    };
}
#endif

#endif
