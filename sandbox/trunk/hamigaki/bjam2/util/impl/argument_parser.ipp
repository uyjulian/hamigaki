// argument_parser.ipp: bjam argument parser implementation

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_IMPL_ARGUMENT_PARSER_IPP
#define HAMIGAKI_BJAM2_UTIL_IMPL_ARGUMENT_PARSER_IPP

#include <hamigaki/bjam2/util/ctype.hpp>
#include <hamigaki/bjam2/util/keywords.hpp>
#include <hamigaki/bjam2/util/punctuators.hpp>
#include <boost/spirit/core.hpp>
#include <string>

namespace hamigaki { namespace bjam2 { namespace impl {

class argument_impl : public boost::spirit::parser<argument_impl>
{
public:
    typedef argument_impl self_t;

    template<class ScannerT>
    struct result
    {
        typedef typename boost::spirit::match_result<
            ScannerT,std::string
        >::type type;
    };

    explicit argument_impl(bool punct_only=false) : punct_only_(punct_only)
    {
    }

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
        bool quoting = false;
        bool may_keyword = true;
        while (!scan.at_end())
        {
            char c = *scan;
            if (!quoting && is_space(c))
                break;
            ++scan.first;
            ++len;

            if (c == '"')
            {
                quoting = !quoting;
                may_keyword = false;
            }
            else if (c == '\\')
            {
                if (scan.at_end())
                    break;

                buf += *scan;
                ++scan.first;
                ++len;
                may_keyword = false;
            }
            else
                buf += c;
        }

        if (scan.first == save)
            return scan.no_match();

        if (may_keyword)
        {
            if (is_lower(buf[0]))
            {
                if (!punct_only_ && bjam2::is_keyword(buf))
                    return scan.no_match();
            }
            else
            {
                if (bjam2::is_punctor(buf))
                    return scan.no_match();
            }
        }

        return scan.create_match(len, buf, save, scan.first);
    }

private:
    bool punct_only_;
};

} } } // End namespaces impl, bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_IMPL_ARGUMENT_PARSER_IPP
