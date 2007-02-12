//  file_attributes.hpp: IEEE P1282 POSIX file attributes

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_POSIX_FILE_ATTRIBUTES_HPP
#define HAMIGAKI_ARCHIVERS_ISO_POSIX_FILE_ATTRIBUTES_HPP

#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso { namespace posix {

struct file_attributes
{
    boost::uint32_t permissions;
    boost::uint32_t links;
    boost::uint32_t uid;
    boost::uint32_t gid;
    boost::uint32_t serial_no;
};

} } } } // End namespaces posix, iso, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ISO_POSIX_FILE_ATTRIBUTES_HPP
