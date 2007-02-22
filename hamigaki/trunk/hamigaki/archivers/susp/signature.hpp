//  signature.hpp: IEEE P1281 System Use Entry signature

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_SUSP_SIGNATURE_HPP
#define HAMIGAKI_ARCHIVERS_SUSP_SIGNATURE_HPP

namespace hamigaki { namespace archivers { namespace susp {

template<char C1, char C2>
struct signature
{
    typedef signature type;

    static const char first_value  = C1;
    static const char second_value = C2;
};

} } } // End namespaces susp, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_SUSP_SIGNATURE_HPP
