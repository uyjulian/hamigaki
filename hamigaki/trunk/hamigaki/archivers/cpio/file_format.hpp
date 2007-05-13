// file_format.hpp: cpio file format

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_CPIO_FILE_FORMAT_HPP
#define HAMIGAKI_ARCHIVERS_CPIO_FILE_FORMAT_HPP

namespace hamigaki { namespace archivers { namespace cpio {

enum file_format
{
    posix, svr4, svr4_chksum, binary
};


} } } // End namespaces cpio, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_CPIO_FILE_FORMAT_HPP
