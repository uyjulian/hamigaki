// file_flags.hpp: ISO 9660 file flags

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_FILE_FLAGS_HPP
#define HAMIGAKI_ARCHIVERS_ISO_FILE_FLAGS_HPP

#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso {

struct file_flags
{
    static const boost::uint8_t existence       = 0x01;
    static const boost::uint8_t directory       = 0x02;
    static const boost::uint8_t associated      = 0x04;
    static const boost::uint8_t record          = 0x08;
    static const boost::uint8_t protection      = 0x10;
    static const boost::uint8_t multi_extent    = 0x80;
};

} } } // End namespaces iso, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ISO_FILE_FLAGS_HPP
