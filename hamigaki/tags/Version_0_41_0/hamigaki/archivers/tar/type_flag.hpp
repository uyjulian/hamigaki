// type_flag.hpp: tar file type flags

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_TAR_TYPE_FLAG_HPP
#define HAMIGAKI_ARCHIVERS_TAR_TYPE_FLAG_HPP

namespace hamigaki { namespace archivers { namespace tar {

struct type_flag
{
    // POSIX.1-1988
    static const char regular       = '0';
    static const char link          = '1';
    static const char symlink       = '2';
    static const char char_device   = '3';
    static const char block_device  = '4';
    static const char directory     = '5';
    static const char fifo          = '6';
    static const char reserved      = '7';

    // GNU extension
    static const char long_link     = 'K';
    static const char long_name     = 'L';

    // POSIX.1-2001
    static const char global        = 'g';
    static const char extended      = 'x';
};

} } } // End namespaces tar, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_TAR_TYPE_FLAG_HPP
