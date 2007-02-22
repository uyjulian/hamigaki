//  pl_parser.hpp: IEEE P1282 "PL" System Use Entry parser

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_RRIP_PL_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_RRIP_PL_PARSER_HPP

#include <hamigaki/binary/endian.hpp>

namespace hamigaki { namespace archivers { namespace rrip {

struct pl_parser
{
    template<class Header>
    bool operator()(Header& head, const char* s, std::size_t size) const
    {
        if (size < 8)
            return true;

#if defined(BOOST_LITTLE_ENDIAN)
        head.data_pos = hamigaki::decode_uint<little,4>(s);
#else
        head.data_pos = hamigaki::decode_uint<big,4>(s+4);
#endif

        return true;
    }
};

} } } // End namespaces rrip, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_RRIP_PL_PARSER_HPP
