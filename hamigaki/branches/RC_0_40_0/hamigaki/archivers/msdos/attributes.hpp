// attributes.hpp: MS-DOS file attributes

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_MSDOS_ATTRIBUTES_HPP
#define HAMIGAKI_ARCHIVERS_MSDOS_ATTRIBUTES_HPP

#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace msdos {

struct attributes
{
    static const boost::uint16_t read_only      = 0x0001;
    static const boost::uint16_t hidden         = 0x0002;
    static const boost::uint16_t system         = 0x0004;
    static const boost::uint16_t volume_label   = 0x0008;
    static const boost::uint16_t directory      = 0x0010;
    static const boost::uint16_t archive        = 0x0020;

    static const boost::uint16_t mask       = 0x003F;
};

} } } // End namespaces msdos, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_MSDOS_ATTRIBUTES_HPP
