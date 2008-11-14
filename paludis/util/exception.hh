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

#ifndef PALUDIS_GUARD_PALUDIS_EXCEPTION_HH
#define PALUDIS_GUARD_PALUDIS_EXCEPTION_HH 1

#include <paludis/util/attributes.hh>
#include <string>
#include <exception>

/** \file
 * Declaration for the Exception base class, the InternalError exception
 * class, the NameError class and related utilities.
 *
 * \ingroup g_exceptions
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Backtrace context class.
     *
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Context
    {
        private:
            Context(const Context &);
            const Context & operator= (const Context &);

        public:
            ///\name Basic operations
            ///\{

            Context(const std::string &);

            ~Context();

            ///\}

            /**
             * Current context.
             */
            static std::string backtrace(const std::string & delim);
    };

    /**
     * Base exception class.
     *
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Exception :
        public std::exception
    {
        private:
            const std::string _message;
            mutable std::string _what_str;
            struct ContextData;
            ContextData * const _context_data;

            const Exception & operator= (const Exception &);

        protected:
            ///\name Basic operations
            ///\{

            Exception(const std::string & message) throw ();

            Exception(const Exception &);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~Exception() throw () PALUDIS_ATTRIBUTE((nothrow));

            ///\}

            /**
             * Return our descriptive error message.
             */
            const std::string & message() const throw () PALUDIS_ATTRIBUTE((nothrow));

            /**
             * Make a backtrace.
             */
            std::string backtrace(const std::string & delim) const;

            /**
             * Is our backtrace empty?
             */
            bool empty() const;

            /**
             * A better what, if possible.
             */
            const char * what() const throw ();
    };

    /**
     * An InternalError is an Exception that is thrown if something that is
     * never supposed to happen happens.
     *
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InternalError :
        public Exception
    {
        public:
            /**
             * Constructor.
             *
             * \param location Should be set to the PALUDIS_HERE macro.
             *
             * \param message A short message.
             */
            InternalError(const std::string & location, const std::string & message) throw ();

            /**
             * Constructor, with no message (deprecated).
             *
             * \param location Should be set to the PALUDIS_HERE macro.
             *
             * \deprecated Use paludis::InternalError::InternalError(const char * const,
             * const std::string &) instead.
             */
            InternalError(const std::string & location) throw () PALUDIS_ATTRIBUTE((deprecated));
    };

    /**
     * A NotAvailableError is an Exception that is thrown if something that is
     * not available (for example due to compile time configure options or platform
     * limitations) is used.
     *
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NotAvailableError :
        public Exception
    {
        public:
            /**
             * Constructor.
             */
            NotAvailableError(const std::string & message) throw ();
    };

    /**
     * A NameError is an Exception that is thrown when some kind of invalid
     * name is encountered.
     *
     * \ingroup g_exceptions
     * \ingroup g_names
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NameError :
        public Exception
    {
        protected:
            /**
             * Constructor.
             *
             * \param name The invalid name encountered.
             * \param role The role for the name, for example "package name".
             */
            NameError(const std::string & name, const std::string & role) throw ();

            /**
             * Constructor.
             *
             * \param name The invalid name encountered.
             * \param role The role for the name, for example "package name".
             * \param msg  Any extra message.
             */
            NameError(const std::string & name, const std::string & role,
                    const std::string & msg) throw ();
    };

    /**
     * A ConfigurationError is thrown when an invalid configuration occurs.
     *
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ConfigurationError :
        public Exception
    {
        public:
            /**
             * Constructor.
             */
            ConfigurationError(const std::string & msg) throw ();
    };

    /** \def PALUDIS_HERE
     * Expands to the current function name, file and line, for use with
     * paludis::InternalError.
     *
     * \ingroup g_exceptions
     */
#define PALUDIS_HERE (std::string(__PRETTY_FUNCTION__) + " at " + \
        std::string(__FILE__) + ":" + paludis::stringify(__LINE__))
}

#endif
