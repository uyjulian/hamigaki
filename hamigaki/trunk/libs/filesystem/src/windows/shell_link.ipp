// shell_link.ipp: the shell link operations for Windows

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include "./shell_link.hpp"

namespace hamigaki { namespace filesystem { namespace detail {

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
HAMIGAKI_FILESYSTEM_DECL error_code
create_shell_link_api(
    const std::wstring& to_ph, const std::wstring& from_ph,
    const wshell_link_options& options)
{
    return detail::create_shell_link_template(to_ph, from_ph, options);
}
#endif

HAMIGAKI_FILESYSTEM_DECL error_code
create_shell_link_api(
    const std::string& to_ph, const std::string& from_ph,
    const shell_link_options& options)
{
    return detail::create_shell_link_template(to_ph, from_ph, options);
}

} } } // End namespaces detail, filesystem, hamigaki.
