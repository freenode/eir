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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_INSTANTIATION_POLICY_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_INSTANTIATION_POLICY_IMPL_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/mutex.hh>

/** \file
 * Implementation for paludis/util/instantiation_policy.hh .
 *
 * \ingroup g_oo
 */

template <typename OurType_>
void
paludis::InstantiationPolicy<OurType_, paludis::instantiation_method::SingletonTag>::_delete(OurType_ * const p)
{
    delete p;
}

template <typename OurType_>
class paludis::InstantiationPolicy<OurType_, paludis::instantiation_method::SingletonTag>::DeleteOnDestruction
{
    private:
        OurType_ * * const _ptr;

    public:
        DeleteOnDestruction(OurType_ * * const p) :
            _ptr(p)
        {
        }

        ~DeleteOnDestruction()
        {
            paludis::InstantiationPolicy<OurType_, paludis::instantiation_method::SingletonTag>::_delete(* _ptr);
            * _ptr = 0;
        }
};

template<typename OurType_>
OurType_ * *
paludis::InstantiationPolicy<OurType_, paludis::instantiation_method::SingletonTag>::_get_instance_ptr()
{
    static OurType_ * instance(0);
    static DeleteOnDestruction delete_instance(&instance);

    return &instance;
}

template<typename OurType_>
OurType_ *
paludis::InstantiationPolicy<OurType_, paludis::instantiation_method::SingletonTag>::get_instance()
{
    OurType_ * * i(_get_instance_ptr());

    if (0 == *i)
    {
        PALUDIS_TLS bool recursive(false);
        if (recursive)
            throw paludis::InternalError(PALUDIS_HERE, "Recursive instantiation");
        Save<bool> save_recursive(&recursive, true);

        static Mutex m;
        Lock l(m);

        i = _get_instance_ptr();
        if (0 == *i)
            *i = new OurType_;
    }

    return *i;
}

template<typename OurType_>
void
paludis::InstantiationPolicy<OurType_, paludis::instantiation_method::SingletonTag>::destroy_instance()
{
    OurType_ * * i(_get_instance_ptr());
    delete *i;
    *i = 0;
}

#endif
