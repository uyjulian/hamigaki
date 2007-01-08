//  joliet_id.hpp: Joliet file/directory ID

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_ID_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_ID_HPP

#include <string>

namespace hamigaki { namespace archivers { namespace detail {

inline int joliet_id_compare(
    const std::string& lhs, const std::string& rhs)
{
    typedef std::string::size_type size_type;

    size_type lhs_size = lhs.size();
    size_type rhs_size = rhs.size();

    if ((lhs_size == 1) && (rhs_size != 1))
        return -1;
    else if ((lhs_size != 1) && (rhs_size == 1))
        return 1;
    else
        return lhs.compare(rhs);
}

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_ID_HPP
