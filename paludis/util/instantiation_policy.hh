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

#ifndef PALUDIS_GUARD_PALUDIS_INSTANTIATION_POLICY_HH
#define PALUDIS_GUARD_PALUDIS_INSTANTIATION_POLICY_HH 1

#include <paludis/util/attributes.hh>

/** \file
 * InstantiationPolicy patterns.
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
     * Instantiation policies for paludis::InstantiationPolicy.
     *
     * \ingroup g_oo
     */
    namespace instantiation_method
    {
        /**
         * Cannot be copied or assigned to.
         *
         * \ingroup g_oo
         */
        struct NonCopyableTag;

        /**
         * Cannot be instantiated
         *
         * \ingroup g_oo
         */
        struct NonInstantiableTag;

        /**
         * Single instance.
         *
         * \ingroup g_oo
         */
        struct SingletonTag;
    }

    /**
     * InstantiationPolicy is used to specify behaviour of classes that have
     * something other than the default C++ instantiation behaviour.
     *
     * \ingroup g_oo
     */
    template <typename OurType_, typename InstantiationMethodTag_>
    struct InstantiationPolicy;

    /**
     * InstantiationPolicy: specialisation for classes that cannot be copied
     * or assigned to.
     *
     * \ingroup g_oo
     * \nosubgrouping
     */
    template<typename OurType_>
    class PALUDIS_VISIBLE InstantiationPolicy<OurType_, instantiation_method::NonCopyableTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);
            const InstantiationPolicy & operator= (const InstantiationPolicy &);

        public:
            InstantiationPolicy()
            {
            }
    };

    /**
     * InstantiationPolicy: specialisation for classes that cannot be created.
     *
     * \ingroup g_oo
     * \nosubgrouping
     */
    template<typename OurType_>
    class InstantiationPolicy<OurType_, instantiation_method::NonInstantiableTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);
            const InstantiationPolicy & operator= (const InstantiationPolicy &);
            InstantiationPolicy();
    };

    /**
     * InstantiationPolicy: specialisation for singleton classes that are
     * created as needed.
     *
     * \ingroup g_oo
     * \nosubgrouping
     */
    template<typename OurType_>
    class PALUDIS_VISIBLE InstantiationPolicy<OurType_, instantiation_method::SingletonTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);

            const InstantiationPolicy & operator= (const InstantiationPolicy &);

            static OurType_ * * _get_instance_ptr();

            class DeleteOnDestruction;
            friend class DeleteOnDestruction;

            static void _delete(OurType_ * const p);

            class DeleteOnDestruction;

        protected:
            ///\name Basic operations
            ///\{

            InstantiationPolicy()
            {
            }

            ///\}

        public:
            ///\name Singleton operations
            ///\{

            /**
             * Fetch our instance.
             */
            static OurType_ * get_instance()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Destroy our instance.
             */
            static void destroy_instance();

            ///\}
    };
}

#endif
