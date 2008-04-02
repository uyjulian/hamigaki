// operations.hpp: the file operations

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_OPERATIONS_HPP
#define HAMIGAKI_FILESYSTEM_OPERATIONS_HPP

#include <hamigaki/filesystem2/detail/config.hpp>
#include <hamigaki/filesystem2/detail/auto_link.hpp>
#include <hamigaki/filesystem2/file_status.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/utility/enable_if.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    #define HAMIGAKI_FS_FUNC(r) \
        template<class Path> \
        inline typename boost::enable_if< \
            boost::filesystem::is_basic_path<Path>, \
            r \
        >::type
    #define HAMIGAKI_FS_TYPENAME typename
#else
    #define HAMIGAKI_FS_FUNC(r) inline r
    typedef boost::filesystem::path Path;
    #define HAMIGAKI_FS_TYPENAME
#endif // defined(BOOST_FILESYSTEM_NARROW_ONLY)

#if defined(BOOST_WINDOWS) && !defined(__GNUC__)
    #pragma comment(lib, "ole32.lib")
#endif

namespace hamigaki { namespace filesystem {

namespace detail
{

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
HAMIGAKI_FILESYSTEM_DECL file_status
status_api(const std::wstring& ph, error_code& ec);

HAMIGAKI_FILESYSTEM_DECL file_status
symlink_status_api(const std::wstring& ph, error_code& ec);


HAMIGAKI_FILESYSTEM_DECL error_code
last_write_time_api(const std::wstring& ph, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL error_code
last_access_time_api(const std::wstring& ph, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL error_code
creation_time_api(const std::wstring& ph, const timestamp& new_time);


HAMIGAKI_FILESYSTEM_DECL
error_code change_attributes_api(
    const std::wstring& ph, file_attributes::value_type attr);

HAMIGAKI_FILESYSTEM_DECL
error_code change_permissions_api(
    const std::wstring& ph, file_permissions::value_type perm);

HAMIGAKI_FILESYSTEM_DECL
error_code change_owner_api(
    const std::wstring& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid);

HAMIGAKI_FILESYSTEM_DECL
error_code change_symlink_owner_api(
    const std::wstring& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid);

#endif // defined(BOOST_FILESYSTEM_NARROW_ONLY)

HAMIGAKI_FILESYSTEM_DECL file_status
status_api(const std::string& ph, error_code& ec);

HAMIGAKI_FILESYSTEM_DECL file_status
symlink_status_api(const std::string& ph, error_code& ec);


HAMIGAKI_FILESYSTEM_DECL error_code
last_write_time_api(const std::string& ph, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL error_code
last_access_time_api(const std::string& ph, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL error_code
creation_time_api(const std::string& ph, const timestamp& new_time);


HAMIGAKI_FILESYSTEM_DECL
error_code change_attributes_api(
    const std::string& ph, file_attributes::value_type attr);

HAMIGAKI_FILESYSTEM_DECL
error_code change_permissions_api(
    const std::string& ph, file_permissions::value_type perm);

HAMIGAKI_FILESYSTEM_DECL
error_code change_owner_api(
    const std::string& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid);

HAMIGAKI_FILESYSTEM_DECL
error_code change_symlink_owner_api(
    const std::string& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid);

} // namespace detail

HAMIGAKI_FS_FUNC(file_status) status(const Path& ph)
{
    error_code ec;
    const file_status& s = detail::status_api(ph.external_file_string(), ec);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::status", ph, ec);
    }
    return s;
}

HAMIGAKI_FS_FUNC(file_status) status(const Path& ph, error_code& ec)
{
    return detail::status_api(ph.external_file_string(), ec);
}

HAMIGAKI_FS_FUNC(file_status) symlink_status(const Path& ph)
{
    error_code ec;
    const file_status& s =
        detail::symlink_status_api(ph.external_file_string(), ec);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::symlink_status", ph, ec);
    }
    return s;
}

HAMIGAKI_FS_FUNC(file_status)
symlink_status(const Path& ph, error_code& ec)
{
    return detail::symlink_status_api(ph.external_file_string(), ec);
}


HAMIGAKI_FS_FUNC(void)
last_write_time(const Path& ph, const timestamp& new_time)
{
    const error_code& ec =
        detail::last_write_time_api(ph.external_file_string(), new_time);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::last_write_time", ph, ec);
    }
}

HAMIGAKI_FS_FUNC(void)
last_access_time(const Path& ph, const timestamp& new_time)
{
    const error_code& ec =
        detail::last_access_time_api(ph.external_file_string(), new_time);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::last_access_time", ph, ec);
    }
}

HAMIGAKI_FS_FUNC(void)
creation_time(const Path& ph, const timestamp& new_time)
{
    const error_code& ec =
        detail::creation_time_api(ph.external_file_string(), new_time);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::creation_time", ph, ec);
    }
}


HAMIGAKI_FS_FUNC(void)
change_attributes(const Path& ph, file_attributes::value_type attr)
{
    const error_code& ec =
        detail::change_attributes_api(ph.external_file_string(), attr);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::change_attributes", ph, ec);
    }
}

HAMIGAKI_FS_FUNC(void)
change_permissions(const Path& ph, file_permissions::value_type perm)
{
    const error_code& ec =
        detail::change_permissions_api(ph.external_file_string(), perm);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::change_permissions", ph, ec);
    }
}

HAMIGAKI_FS_FUNC(void)
change_owner(
    const Path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    const error_code& ec =
        detail::change_owner_api(ph.external_file_string(), new_uid, new_gid);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::change_owner", ph, ec);
    }
}

HAMIGAKI_FS_FUNC(void)
change_symlink_owner(
    const Path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    const error_code& ec =
        detail::change_symlink_owner_api(
            ph.external_file_string(), new_uid, new_gid
        );
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::change_symlink_owner", ph, ec);
    }
}

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
inline file_status status(const path& ph)
{
    return hamigaki::filesystem::status<path>(ph);
}
inline file_status status(const wpath& ph)
{
    return hamigaki::filesystem::status<wpath>(ph);
}

inline file_status status(const path& ph, error_code& ec)
{
    return hamigaki::filesystem::status<path>(ph, ec);
}
inline file_status status(const wpath& ph, error_code& ec)
{
    return hamigaki::filesystem::status<wpath>(ph, ec);
}

inline file_status symlink_status(const path& ph)
{
    return hamigaki::filesystem::symlink_status<path>(ph);
}
inline file_status symlink_status(const wpath& ph)
{
    return hamigaki::filesystem::symlink_status<wpath>(ph);
}

inline file_status symlink_status(const path& ph, error_code& ec)
{
    return hamigaki::filesystem::symlink_status<path>(ph, ec);
}
inline file_status symlink_status(const wpath& ph, error_code& ec)
{
    return hamigaki::filesystem::symlink_status<wpath>(ph, ec);
}


inline bool exists(const path& ph)
{
    return exists(hamigaki::filesystem::status(ph));
}
inline bool exists(const wpath& ph)
{
    return exists(hamigaki::filesystem::status(ph));
}

inline bool is_directory(const path& ph)
{
    return is_directory(hamigaki::filesystem::status(ph));
}
inline bool is_directory(const wpath& ph)
{
    return is_directory(hamigaki::filesystem::status(ph));
}

inline bool is_regular(const path& ph)
{
    return is_regular(hamigaki::filesystem::status(ph));
}
inline bool is_regular(const wpath& ph)
{
    return is_regular(hamigaki::filesystem::status(ph));
}

inline bool is_other(const path& ph)
{
    return is_other(hamigaki::filesystem::status(ph));
}
inline bool is_other(const wpath& ph)
{
    return is_other(hamigaki::filesystem::status(ph));
}

inline bool is_symlink(const path& ph)
{
    return is_symlink(hamigaki::filesystem::symlink_status(ph));
}
inline bool is_symlink(const wpath& ph)
{
    return is_symlink(hamigaki::filesystem::symlink_status(ph));
}


inline void last_write_time(const path& ph, const timestamp& new_time)
{
    return hamigaki::filesystem::last_write_time<path>(ph, new_time);
}
inline void last_write_time(const wpath& ph, const timestamp& new_time)
{
    return hamigaki::filesystem::last_write_time<wpath>(ph, new_time);
}

inline void last_access_time(const path& ph, const timestamp& new_time)
{
    return hamigaki::filesystem::last_write_time<path>(ph, new_time);
}
inline void last_access_time(const wpath& ph, const timestamp& new_time)
{
    return hamigaki::filesystem::last_write_time<wpath>(ph, new_time);
}

inline void creation_time(const path& ph, const timestamp& new_time)
{
    return hamigaki::filesystem::creation_time<path>(ph, new_time);
}
inline void creation_time(const wpath& ph, const timestamp& new_time)
{
    return hamigaki::filesystem::creation_time<wpath>(ph, new_time);
}


inline void change_attributes(const path& ph, file_attributes::value_type attr)
{
    return hamigaki::filesystem::change_attributes<path>(ph, attr);
}
inline void change_attributes(const wpath& ph, file_attributes::value_type attr)
{
    return hamigaki::filesystem::change_attributes<wpath>(ph, attr);
}

inline void change_permissions(
    const path& ph, file_permissions::value_type perm)
{
    return hamigaki::filesystem::change_permissions<path>(ph, perm);
}
inline void change_permissions(
    const wpath& ph, file_permissions::value_type perm)
{
    return hamigaki::filesystem::change_permissions<wpath>(ph, perm);
}

inline void change_owner(
    const path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    return hamigaki::filesystem::change_owner<path>(ph, new_uid, new_gid);
}
inline void change_owner(
    const wpath& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    return hamigaki::filesystem::change_owner<wpath>(ph, new_uid, new_gid);
}

inline void change_symlink_owner(
    const path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    return hamigaki::filesystem::
        change_symlink_owner<path>(ph, new_uid, new_gid);
}
inline void change_symlink_owner(
    const wpath& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    return hamigaki::filesystem::
        change_symlink_owner<wpath>(ph, new_uid, new_gid);
}

#endif // defined(BOOST_FILESYSTEM_NARROW_ONLY)

} } // End namespaces filesystem, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_FILESYSTEM_OPERATIONS_HPP
