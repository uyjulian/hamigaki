//  re_parser.hpp: IEEE P1282 "RE" System Use Entry parser

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_RRIP_RE_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_RRIP_RE_PARSER_HPP

#include <cstddef>

namespace hamigaki { namespace archivers { namespace rrip {

struct re_parser
{
    template<class Header>
    bool operator()(Header&, const char*, std::size_t) const
    {
        return false;
    }
};

} } } // End namespaces rrip, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_RRIP_RE_PARSER_HPP
