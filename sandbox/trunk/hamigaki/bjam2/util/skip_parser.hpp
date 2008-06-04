// skip_parser.hpp: bjam skip parser

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_SKIP_PARSER_HPP
#define HAMIGAKI_BJAM2_UTIL_SKIP_PARSER_HPP

#include <boost/spirit/core.hpp>
#include <cctype>

namespace hamigaki { namespace bjam2 {

struct skip_parser : public boost::spirit::parser<skip_parser>
{
    typedef skip_parser self_t;

    template<class ScannerT>
    struct result
    {
        typedef typename boost::spirit::match_result<
            ScannerT, boost::spirit::nil_t
        >::type type;
    };

    template<class ScannerT>
    typename boost::spirit::parser_result<self_t,ScannerT>::type
    parse(const ScannerT& scan) const
    {
        using namespace boost::spirit;

        typedef typename ScannerT::iterator_t iterator_t;

        iterator_t save(scan.first);
        std::size_t len = 0;

        if (!scan.at_end())
        {
            char c = *scan;
            if (std::isspace(c))
            {
                ++scan.first;
                ++len;
            }
            else if (c == '#')
            {
                ++scan.first;
                ++len;

                while (!scan.at_end())
                {
                    char c = *scan;

                    ++scan.first;
                    ++len;

                    if (c == '\n')
                        break;
                }
            }
        }

        if (scan.first == save)
            return scan.no_match();

        return scan.create_match(len, nil_t(), save, scan.first);
    }
};

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_SKIP_PARSER_HPP
