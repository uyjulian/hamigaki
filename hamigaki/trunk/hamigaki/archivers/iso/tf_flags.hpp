//  tf_flags.hpp: IEEE P1282 "TF" flags

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO9660_TF_FLAGS_HPP
#define HAMIGAKI_ARCHIVERS_ISO9660_TF_FLAGS_HPP

#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso9660 {

struct tf_flags
{
    static const boost::uint8_t creation        = 0x01;
    static const boost::uint8_t modify          = 0x02;
    static const boost::uint8_t access          = 0x04;
    static const boost::uint8_t attributes      = 0x08;
    static const boost::uint8_t backup          = 0x10;
    static const boost::uint8_t expiration      = 0x20;
    static const boost::uint8_t effective       = 0x40;
    static const boost::uint8_t long_form       = 0x80;
};

} } } // End namespaces iso9660, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ISO9660_TF_FLAGS_HPP
