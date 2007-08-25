// argument_parser.hpp: bjam argument parser

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_ARGUMENT_PARSER_HPP
#define HAMIGAKI_BJAM_UTIL_ARGUMENT_PARSER_HPP

#include <hamigaki/bjam/util/impl/argument_parser.ipp>

namespace hamigaki { namespace bjam {

class argument : public boost::spirit::parser<argument>
{
public:
    typedef argument self_t;

    template<class ScannerT>
    struct result
    {
        typedef typename boost::spirit::match_result<
            ScannerT,std::string
        >::type type;
    };

    explicit argument(bool punct_only=false) : impl_(punct_only)
    {
    }

    template<class ScannerT>
    typename boost::spirit::parser_result<self_t,ScannerT>::type
    parse(const ScannerT& scan) const
    {
        return boost::spirit::lexeme_d[impl_].parse(scan);
    }

private:
    impl::argument_impl impl_;
};

const argument arg_p = argument();
const argument non_punct_p = argument(true);

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_ARGUMENT_PARSER_HPP
