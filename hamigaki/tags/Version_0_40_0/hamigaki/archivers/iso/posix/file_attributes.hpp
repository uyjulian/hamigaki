// file_attributes.hpp: IEEE P1282 POSIX file attributes

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_POSIX_FILE_ATTRIBUTES_HPP
#define HAMIGAKI_ARCHIVERS_ISO_POSIX_FILE_ATTRIBUTES_HPP

#include <boost/cstdint.hpp>
#include <boost/operators.hpp>

namespace hamigaki { namespace archivers { namespace iso { namespace posix {

struct file_attributes : boost::equality_comparable<file_attributes>
{
    boost::uint32_t permissions;
    boost::uint32_t links;
    boost::uint32_t uid;
    boost::uint32_t gid;
    boost::uint32_t serial_no;

    file_attributes()
        : permissions(0100644u), links(1u), uid(0u), gid(0u), serial_no(0u)
    {
    }

    bool operator==(const file_attributes& rhs) const
    {
        return
            (permissions    == rhs.permissions  ) &&
            (links          == rhs.links        ) &&
            (uid            == rhs.uid          ) &&
            (gid            == rhs.gid          ) &&
            (serial_no      == rhs.serial_no    ) ;
    }
};

} } } } // End namespaces posix, iso, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ISO_POSIX_FILE_ATTRIBUTES_HPP
