// skip_parser.hpp: bjam skip parser

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_SKIP_PARSER_HPP
#define HAMIGAKI_BJAM_UTIL_SKIP_PARSER_HPP

#include <boost/spirit/core.hpp>

namespace hamigaki { namespace bjam {

struct skip_parser : public boost::spirit::parser<skip_parser>
{
    typedef skip_parser self_t;

    template<class ScannerT>
    typename boost::spirit::parser_result<self_t,ScannerT>::type
    parse(const ScannerT& scan) const
    {
        using namespace boost::spirit;

        return
            (   space_p
            |   '#' >> *(anychar_p - '\n') >> '\n'
            ).parse(scan);
    }
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_SKIP_PARSER_HPP
