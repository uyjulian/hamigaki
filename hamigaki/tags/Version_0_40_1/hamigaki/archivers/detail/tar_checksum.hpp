// tar_checksum.hpp: POSIX.1-1988 tar checksum

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_TAR_CHECKSUM_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_TAR_CHECKSUM_HPP

#include <boost/config.hpp>

#include <hamigaki/archivers/tar/raw_header.hpp>
#include <boost/integer.hpp>
#include <cstring>
#include <numeric>

namespace hamigaki { namespace archivers { namespace detail {

typedef boost::uint_t<17>::least uint17_t;

inline uint17_t tar_checksum(const void* block)
{
    using tar::raw_header;

    const unsigned char* s = static_cast<const unsigned char*>(block);

    static const std::size_t chksum_offset =
        binary_offset<raw_header,char[8],&raw_header::chksum>::value;

    static const std::size_t typeflag_offset =
        binary_offset<raw_header,char,&raw_header::typeflag>::value;

    uint17_t chksum = 0;
    chksum = std::accumulate(s, s+chksum_offset, chksum);
    chksum += static_cast<uint17_t>(static_cast<unsigned char>(' ') * 8u);
    chksum = std::accumulate(
        s+typeflag_offset, s+raw_header::block_size, chksum);
    return chksum;
}

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_TAR_CHECKSUM_HPP
