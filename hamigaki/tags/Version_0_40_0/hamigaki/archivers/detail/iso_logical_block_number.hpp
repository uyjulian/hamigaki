// iso_logical_block_number.hpp: utility for ISO 9660 logical block number

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_LOGICAL_BLOCK_NUMBER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_LOGICAL_BLOCK_NUMBER_HPP

#include <boost/config.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace detail {

inline boost::uint32_t calc_lbn_shift(boost::uint16_t block_size)
{
    if (block_size == 2048)
        return 11;
    else if (block_size == 1024)
        return 10;
    else if (block_size == 512)
        return 9;
    else
        throw BOOST_IOSTREAMS_FAILURE("invalid ISO 9660 logical block size");

    BOOST_UNREACHABLE_RETURN(0)
}

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_LOGICAL_BLOCK_NUMBER_HPP
