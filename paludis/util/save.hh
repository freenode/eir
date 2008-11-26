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

#ifndef PALUDIS_GUARD_PALUDIS_SAVE_HH
#define PALUDIS_GUARD_PALUDIS_SAVE_HH 1

#include <paludis/util/attributes.hh>
#include <tr1/functional>

/** \file
 * Declarations for the Save and RunOnDestruction classes.
 *
 * \ingroup g_utils
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Save the value of a particular variable and assign it a new value for the
     * duration of the Save instance's lifetime (RAII, see \ref EffCpp item 13 or
     * \ref TCppPL section 14.4).
     *
     * \ingroup g_utils
     * \nosubgrouping
     */
    template <typename T_>
    class Save
    {
        private:
            T_ * const _ptr;
            const T_ _value;

            Save(const Save &);
            void operator= (const Save &);

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor.
             */
            Save(T_ * const p) :
                _ptr(p),
                _value(*p)
            {
            }

            /**
             * Constructor, with convenience assignment to new_value.
             */
            Save(T_ * const p, const T_ & new_value) :
                _ptr(p),
                _value(*p)
            {
                *p = new_value;
            }

            /**
             * Destructor.
             */
            ~Save()
            {
                *_ptr = _value;
            }

            ///\}
    };

    /**
     * Run the supplied function when the class is destructed.
     *
     * \ingroup g_utils
     * \since 0.26
     */
    class PALUDIS_VISIBLE RunOnDestruction
    {
        private:
            RunOnDestruction(const RunOnDestruction &);
            void operator= (const RunOnDestruction &);

            const std::tr1::function<void ()> _f;

        public:
            RunOnDestruction(const std::tr1::function<void ()> & f) :
                _f(f)
            {
            }

            ~RunOnDestruction()
            {
                _f();
            }
    };

}

#endif

