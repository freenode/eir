/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_TOKENISER_HH
#define PALUDIS_GUARD_PALUDIS_TOKENISER_HH 1

#include <iterator>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <string>

/** \file
 * Declarations for Tokeniser and related utilities.
 *
 * \ingroup g_strings
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Delimiter policy for Tokeniser.
     *
     * \ingroup g_strings
     */
    namespace delim_kind
    {
        /**
         * Any of the characters split, and the delimiter is discarded.
         *
         * \ingroup g_strings
         */
        struct AnyOfTag;
    }

    /**
     * Delimiter mode for Tokeniser.
     *
     * \ingroup g_strings
     */
    namespace delim_mode
    {
        /**
         * Discard the delimiters.
         *
         * \ingroup g_strings
         */
        struct DelimiterTag;

        /**
         * Keep the delimiters.
         *
         * \ingroup g_strings
         */
        struct BoundaryTag;
    }

    /**
     * Tokeniser internal use only.
     *
     * \ingroup g_strings
     */
    namespace tokeniser_internals
    {
        /**
         * A Writer handles Tokeniser's writes.
         *
         * \ingroup g_strings
         */
        template <typename DelimMode_, typename Iter_>
        struct Writer;

        /**
         * A Writer handles Tokeniser's writes (specialisation for
         * delim_mode::DelimiterTag).
         *
         * \ingroup g_strings
         */
        template <typename Iter_>
        struct Writer<delim_mode::DelimiterTag, Iter_>
        {
            /**
             * Handle a token.
             */
            static void handle_token(const std::string & s, Iter_ & i)
            {
                *i++ = s;
            }

            /**
             * Handle a delimiter.
             */
            static void handle_delim(const std::string &, const Iter_ &)
            {
            }
        };

        /**
         * A Writer handles Tokeniser's writes (specialisation for
         * delim_mode::BoundaryTag).
         *
         * \ingroup g_strings
         */
        template <typename Iter_>
        struct Writer<delim_mode::BoundaryTag, Iter_>
        {
            /**
             * Handle a token.
             */
            static void handle_token(const std::string & s, Iter_ & i)
            {
                *i++ = s;
            }

            /**
             * Handle a delimiter.
             */
            static void handle_delim(const std::string & s, Iter_ & i)
            {
                *i++ = s;
            }
        };

        struct Lexer
        {
            const std::string text;
            std::string::size_type text_pos;
            std::string delims;
            const std::string quotes;

            std::string value;
            enum { t_quote, t_delim, t_text } kind;

            Lexer(const std::string & t, const std::string & d, const std::string & q) :
                text(t),
                text_pos(0),
                delims(d),
                quotes(q)
            {
            }

            bool next()
            {
                if (text_pos >= text.length())
                    return false;

                if (std::string::npos != delims.find(text[text_pos]))
                {
                    std::string::size_type start_pos(text_pos);
                    while (++text_pos < text.length())
                        if (std::string::npos == delims.find(text[text_pos]))
                            break;

                    value = text.substr(start_pos, text_pos - start_pos);
                    kind = t_delim;
                }
                else if (std::string::npos != quotes.find(text[text_pos]))
                {
                    value = std::string(1, text[text_pos]);
                    kind = t_quote;
                    ++text_pos;
                }
                else
                {
                    std::string::size_type start_pos(text_pos);
                    while (++text_pos < text.length())
                        if (std::string::npos != delims.find(text[text_pos]))
                            break;
                        else if (std::string::npos != quotes.find(text[text_pos]))
                            break;
                    value = text.substr(start_pos, text_pos - start_pos);
                    kind = t_text;
                }

                return true;
            }
        };

        template <typename DelimKind_, typename DelimMode_ = delim_mode::DelimiterTag>
        struct Tokeniser;

        template <typename DelimMode_>
        class Tokeniser<delim_kind::AnyOfTag, DelimMode_>
        {
            private:
                Tokeniser();

            public:
                template <typename Iter_>
                static void tokenise(const std::string & s,
                        const std::string & delims,
                        const std::string & quotes,
                        Iter_ iter);
        };
    }

    /**
     * Thrown if a Tokeniser encounters a syntax error (for example, mismatched quotes).
     *
     * \ingroup g_strings
     * \since 0.26
     */
    class PALUDIS_VISIBLE TokeniserError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            TokeniserError(const std::string & s, const std::string & msg) throw ();

            ///\}
    };

    template <typename DelimMode_>
    template <typename Iter_>
    void
    tokeniser_internals::Tokeniser<delim_kind::AnyOfTag, DelimMode_>::tokenise(
            const std::string & s,
            const std::string & delims,
            const std::string & quotes,
            Iter_ iter)
    {
        typedef tokeniser_internals::Lexer Lexer;
        Lexer l(s, delims, quotes);

        enum { s_initial, s_had_quote, s_had_text, s_had_quote_text, s_had_quote_text_quote } state = s_initial;

        while (l.next())
        {
            switch (state)
            {
                case s_initial:
                    switch (l.kind)
                    {
                        case Lexer::t_quote:
                            state = s_had_quote;
                            l.delims = "";
                            break;

                        case Lexer::t_delim:
                            state = s_initial;
                            tokeniser_internals::Writer<DelimMode_, Iter_>::handle_delim(l.value, iter);
                            break;

                        case Lexer::t_text:
                            state = s_had_text;
                            tokeniser_internals::Writer<DelimMode_, Iter_>::handle_token(l.value, iter);
                            break;
                    }
                    break;

                case s_had_quote:
                    switch (l.kind)
                    {
                        case Lexer::t_quote:
                            state = s_had_quote_text_quote;
                            l.delims = delims;
                            tokeniser_internals::Writer<DelimMode_, Iter_>::handle_token("", iter);
                            break;

                        case Lexer::t_delim:
                            throw InternalError(PALUDIS_HERE, "t_delim in s_had_quote");
                            break;

                        case Lexer::t_text:
                            state = s_had_quote_text;
                            tokeniser_internals::Writer<DelimMode_, Iter_>::handle_token(l.value, iter);
                            break;
                    }
                    break;

                case s_had_quote_text:
                    switch (l.kind)
                    {
                        case Lexer::t_text:
                            throw InternalError(PALUDIS_HERE, "t_text in s_had_quote_text");
                            break;

                        case Lexer::t_delim:
                            throw InternalError(PALUDIS_HERE, "t_delim in s_had_quote_text");
                            break;

                        case Lexer::t_quote:
                            state = s_had_quote_text_quote;
                            l.delims = delims;
                            break;
                    }
                    break;

                case s_had_quote_text_quote:
                    switch (l.kind)
                    {
                        case Lexer::t_text:
                            throw TokeniserError(s, "Close quote followed by text");
                            break;

                        case Lexer::t_quote:
                            throw TokeniserError(s, "Close quote followed by quote");
                            break;

                        case Lexer::t_delim:
                            state = s_initial;
                            tokeniser_internals::Writer<DelimMode_, Iter_>::handle_delim(l.value, iter);
                            break;
                    }
                    break;

                case s_had_text:
                    switch (l.kind)
                    {
                        case Lexer::t_text:
                            throw InternalError(PALUDIS_HERE, "t_text in s_had_text");
                            break;

                        case Lexer::t_quote:
                            throw TokeniserError(s, "Text followed by quote");
                            break;

                        case Lexer::t_delim:
                            state = s_initial;
                            tokeniser_internals::Writer<DelimMode_, Iter_>::handle_delim(l.value, iter);
                            break;
                    }
                    break;
            }
        }

        switch (state)
        {
            case s_initial:
            case s_had_text:
            case s_had_quote_text_quote:
                return;

            case s_had_quote:
            case s_had_quote_text:
                throw TokeniserError(s, "Unterminated quoted string");
        }
    }

    /**
     * Tokenise a string.
     *
     * \ingroup g_strings
     * \since 0.26
     */
    template <typename DelimKind_, typename DelimMode_, typename Iter_>
    void tokenise(const std::string & s, const std::string & delims, const std::string & quotes, Iter_ iter)
    {
        tokeniser_internals::Tokeniser<DelimKind_, DelimMode_>::template tokenise<Iter_>(s, delims, quotes, iter);
    }

    /**
     * Convenience function: tokenise on whitespace.
     *
     * \ingroup g_strings
     * \since 0.26
     */
    template <typename Iter_>
    void tokenise_whitespace(const std::string & s, Iter_ iter)
    {
        tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(s, " \t\r\n", "", iter);
    }

    /**
     * Convenience function: tokenise on whitespace, handling quoted strings.
     *
     * \ingroup g_strings
     * \since 0.26
     */
    template <typename Iter_>
    void tokenise_whitespace_quoted(const std::string &s, Iter_ iter)
    {
        tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(s, " \t\r\n", "'\"", iter);
    }
}

#endif
