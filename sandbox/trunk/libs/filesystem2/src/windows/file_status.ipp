// file_status.ipp: the file status operations for Windows

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include "./helpers.hpp"

namespace hamigaki { namespace filesystem { namespace detail {

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
HAMIGAKI_FILESYSTEM_DECL file_status
status_api(const std::wstring& ph, error_code& ec)
{
    return detail::status_template(ph, ec);
}

HAMIGAKI_FILESYSTEM_DECL file_status
symlink_status_api(const std::wstring& ph, error_code& ec)
{
    return detail::symlink_status_template(ph, ec);
}


HAMIGAKI_FILESYSTEM_DECL error_code
last_write_time_api(const std::wstring& ph, const timestamp& new_time)
{
    return detail::last_write_time_template(ph, new_time);
}

HAMIGAKI_FILESYSTEM_DECL error_code
last_access_time_api(const std::wstring& ph, const timestamp& new_time)
{
    return detail::last_access_time_template(ph, new_time);
}

HAMIGAKI_FILESYSTEM_DECL error_code
creation_time_api(const std::wstring& ph, const timestamp& new_time)
{
    return detail::creation_time_template(ph, new_time);
}


HAMIGAKI_FILESYSTEM_DECL
error_code change_attributes_api(
    const std::wstring& ph, file_attributes::value_type attr)
{
    return change_attributes_template(ph, attr);
}

HAMIGAKI_FILESYSTEM_DECL
error_code change_permissions_api(
    const std::wstring&, file_permissions::value_type)
{
    return make_error_code(ERROR_NOT_SUPPORTED);
}

HAMIGAKI_FILESYSTEM_DECL
error_code change_owner_api(
    const std::wstring&,
    const boost::optional<boost::intmax_t>&,
    const boost::optional<boost::intmax_t>&)
{
    return make_error_code(ERROR_NOT_SUPPORTED);
}

HAMIGAKI_FILESYSTEM_DECL
error_code change_symlink_owner_api(
    const std::wstring&,
    const boost::optional<boost::intmax_t>&,
    const boost::optional<boost::intmax_t>&)
{
    return make_error_code(ERROR_NOT_SUPPORTED);
}
#endif

HAMIGAKI_FILESYSTEM_DECL file_status
status_api(const std::string& ph, error_code& ec)
{
    return detail::status_template(ph, ec);
}

HAMIGAKI_FILESYSTEM_DECL file_status
symlink_status_api(const std::string& ph, error_code& ec)
{
    return detail::symlink_status_template(ph, ec);
}

HAMIGAKI_FILESYSTEM_DECL error_code
last_write_time_api(const std::string& ph, const timestamp& new_time)
{
    return detail::last_write_time_template(ph, new_time);
}

HAMIGAKI_FILESYSTEM_DECL error_code
last_access_time_api(const std::string& ph, const timestamp& new_time)
{
    return detail::last_access_time_template(ph, new_time);
}

HAMIGAKI_FILESYSTEM_DECL error_code
creation_time_api(const std::string& ph, const timestamp& new_time)
{
    return detail::creation_time_template(ph, new_time);
}


HAMIGAKI_FILESYSTEM_DECL
error_code change_attributes_api(
    const std::string& ph, file_attributes::value_type attr)
{
    return change_attributes_template(ph, attr);
}

HAMIGAKI_FILESYSTEM_DECL
error_code change_permissions_api(
    const std::string&, file_permissions::value_type)
{
    return make_error_code(ERROR_NOT_SUPPORTED);
}

HAMIGAKI_FILESYSTEM_DECL
error_code change_owner_api(
    const std::string&,
    const boost::optional<boost::intmax_t>&,
    const boost::optional<boost::intmax_t>&)
{
    return make_error_code(ERROR_NOT_SUPPORTED);
}

HAMIGAKI_FILESYSTEM_DECL
error_code change_symlink_owner_api(
    const std::string&,
    const boost::optional<boost::intmax_t>&,
    const boost::optional<boost::intmax_t>&)
{
    return make_error_code(ERROR_NOT_SUPPORTED);
}

} } } // End namespaces detail, filesystem, hamigaki.
