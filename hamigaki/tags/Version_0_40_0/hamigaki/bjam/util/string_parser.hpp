// string_parser.hpp: bjam string parser

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_STRING_PARSER_HPP
#define HAMIGAKI_BJAM_UTIL_STRING_PARSER_HPP

#include <hamigaki/bjam/util/impl/string_parser.ipp>

namespace hamigaki { namespace bjam {

struct string_parser : public boost::spirit::parser<string_parser>
{
    typedef string_parser self_t;

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
        return boost::spirit::lexeme_d[impl::string_parser_impl()].parse(scan);
    }
};

const string_parser string_p = string_parser();

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_STRING_PARSER_HPP
