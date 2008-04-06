// error_code.hpp: the utility function for error_code

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_WINDOWS_ERROR_CODE_HPP
#define HAMIGAKI_FILESYSTEM_WINDOWS_ERROR_CODE_HPP

#include <hamigaki/filesystem/detail/config.hpp>
#include <windows.h>

namespace hamigaki { namespace filesystem { namespace detail {

inline error_code last_error()
{
    return make_error_code(static_cast<int>(::GetLastError()));
}

} } } // End namespaces detail, filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_WINDOWS_ERROR_CODE_HPP
