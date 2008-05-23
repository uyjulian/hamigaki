// string_parser.ipp: bjam string parser implementation

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_IMPL_STRING_PARSER_IPP
#define HAMIGAKI_BJAM2_UTIL_IMPL_STRING_PARSER_IPP

#include <hamigaki/bjam/util/keywords.hpp>
#include <hamigaki/bjam/util/punctuators.hpp>
#include <boost/spirit/core.hpp>
#include <cctype>
#include <string>

namespace hamigaki { namespace bjam2 { namespace impl {

struct string_parser_impl : public boost::spirit::parser<string_parser_impl>
{
    typedef string_parser_impl self_t;

    template<class ScannerT>
    struct result
    {
        typedef typename boost::spirit::match_result<
            ScannerT,std::string
        >::type type;
    };

    template<class ScannerT>
    typename boost::spirit::parser_result<self_t,ScannerT>::type
    parse(const ScannerT& scan) const
    {
        using namespace boost::spirit;

        typedef typename parser_result<self_t,ScannerT>::type result_t;
        typedef typename ScannerT::iterator_t iterator_t;

        iterator_t save(scan.first);

        std::string buf;
        std::size_t len = 0;
        std::size_t nest = 1;
        while (!scan.at_end())
        {
            char c = *scan;
            if ((c == '}') && (--nest == 0))
                break;
            ++scan.first;
            ++len;

            if (c == '{')
                ++nest;

            if ((c == '\n') && !buf.empty() && (*buf.rbegin() == '\r'))
                *buf.rbegin() = '\n';
            else
                buf += c;
        }

        if (nest != 0)
            return scan.no_match();

        return scan.create_match(len, buf, save, scan.first);
    }
};

} } } // End namespaces impl, bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_IMPL_STRING_PARSER_IPP
