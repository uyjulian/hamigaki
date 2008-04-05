// symlink.ipp: the symbolic link operations for Windows

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include "./helpers.hpp"

namespace hamigaki { namespace filesystem { namespace detail {

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
HAMIGAKI_FILESYSTEM_DECL error_code
symlink_target_api(const std::wstring& ph, std::wstring& target)
{
    return detail::symlink_target_template(ph, target);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_hard_link_api(
    const std::wstring& to_ph, const std::wstring& from_ph)
{
    return detail::create_hard_link_template(to_ph, from_ph);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_file_symlink_api(
    const std::wstring& to_ph, const std::wstring& from_ph)
{
    return detail::create_file_symlink_template(to_ph, from_ph);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_directory_symlink_api(
    const std::wstring& to_ph, const std::wstring& from_ph)
{
    return detail::create_directory_symlink_template(to_ph, from_ph);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_symlink_api(const std::wstring& to_ph, const std::wstring& from_ph)
{
    return detail::create_symlink_template(to_ph, from_ph);
}
#endif

HAMIGAKI_FILESYSTEM_DECL error_code
symlink_target_api(const std::string& ph, std::string& target)
{
    return detail::symlink_target_template(ph, target);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_hard_link_api(
    const std::string& to_ph, const std::string& from_ph)
{
    return detail::create_hard_link_template(to_ph, from_ph);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_file_symlink_api(
    const std::string& to_ph, const std::string& from_ph)
{
    return detail::create_file_symlink_template(to_ph, from_ph);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_directory_symlink_api(
    const std::string& to_ph, const std::string& from_ph)
{
    return detail::create_directory_symlink_template(to_ph, from_ph);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_symlink_api(const std::string& to_ph, const std::string& from_ph)
{
    return detail::create_symlink_template(to_ph, from_ph);
}

} } } // End namespaces detail, filesystem, hamigaki.
