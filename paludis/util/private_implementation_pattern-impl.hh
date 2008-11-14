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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_PRIVATE_IMPLEMENTATION_PATTERN_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_PRIVATE_IMPLEMENTATION_PATTERN_IMPL_HH 1

#include <paludis/util/private_implementation_pattern.hh>

/** \file
 * Implementation for paludis/util/private_implementation_pattern.hh .
 *
 * \ingroup g_oo
 */

template <typename C_>
paludis::PrivateImplementationPattern<C_>::ImpPtr::ImpPtr(Implementation<C_> * p) :
    _ptr(p)
{
}

template <typename C_>
paludis::PrivateImplementationPattern<C_>::ImpPtr::~ImpPtr()
{
    delete _ptr;
}

template <typename C_>
paludis::Implementation<C_> *
paludis::PrivateImplementationPattern<C_>::ImpPtr::get()
{
    return _ptr;
}

template <typename C_>
const paludis::Implementation<C_> *
paludis::PrivateImplementationPattern<C_>::ImpPtr::get() const
{
    return _ptr;
}

template <typename C_>
void
paludis::PrivateImplementationPattern<C_>::ImpPtr::reset(Implementation<C_> * p)
{
    delete _ptr;
    _ptr = p;
}

template <typename C_>
paludis::PrivateImplementationPattern<C_>::PrivateImplementationPattern(Implementation<C_> * i) :
    _imp(i)
{
}

template <typename C_>
paludis::PrivateImplementationPattern<C_>::~PrivateImplementationPattern()
{
}

#endif
