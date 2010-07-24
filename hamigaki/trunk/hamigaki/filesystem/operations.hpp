// operations.hpp: the file operations

// Copyright Takeshi Mouri 2006-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_OPERATIONS_HPP
#define HAMIGAKI_FILESYSTEM_OPERATIONS_HPP

#include <hamigaki/filesystem/detail/config.hpp>
#include <hamigaki/filesystem/detail/auto_link.hpp>
#include <hamigaki/filesystem/file_status.hpp>
#include <hamigaki/filesystem/shell_link_options.hpp>
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
            boost::HAMIGAKI_BOOST_FS_NAMESPACE::is_basic_path<Path>, \
            r \
        >::type
    #define HAMIGAKI_FS_TYPENAME typename
    #define HAMIGAKI_FS_SPEC <Path>
#else
    #define HAMIGAKI_FS_FUNC(r) inline r
    typedef boost::filesystem::path Path;
    #define HAMIGAKI_FS_TYPENAME
    #define HAMIGAKI_FS_SPEC
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
symlink_target_api(const std::wstring& ph, std::wstring& target);

HAMIGAKI_FILESYSTEM_DECL error_code
last_write_time_api(const std::wstring& ph, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL error_code
last_access_time_api(const std::wstring& ph, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL error_code
creation_time_api(const std::wstring& ph, const timestamp& new_time);


HAMIGAKI_FILESYSTEM_DECL error_code
create_hard_link_api(const std::wstring& to_ph, const std::wstring& from_ph);

HAMIGAKI_FILESYSTEM_DECL error_code
create_file_symlink_api(const std::wstring& to_ph, const std::wstring& from_ph);

HAMIGAKI_FILESYSTEM_DECL error_code
create_directory_symlink_api(
    const std::wstring& to_ph, const std::wstring& from_ph);

HAMIGAKI_FILESYSTEM_DECL error_code
create_symlink_api(const std::wstring& to_ph, const std::wstring& from_ph);

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

#if defined(BOOST_WINDOWS)
HAMIGAKI_FILESYSTEM_DECL error_code
create_shell_link_api(
    const std::wstring& to_ph, const std::wstring& from_ph,
    const wshell_link_options& options);
#endif // defined(BOOST_WINDOWS)

#endif // !defined(BOOST_FILESYSTEM_NARROW_ONLY)

HAMIGAKI_FILESYSTEM_DECL file_status
status_api(const std::string& ph, error_code& ec);

HAMIGAKI_FILESYSTEM_DECL file_status
symlink_status_api(const std::string& ph, error_code& ec);


HAMIGAKI_FILESYSTEM_DECL error_code
symlink_target_api(const std::string& ph, std::string& target);

HAMIGAKI_FILESYSTEM_DECL error_code
last_write_time_api(const std::string& ph, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL error_code
last_access_time_api(const std::string& ph, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL error_code
creation_time_api(const std::string& ph, const timestamp& new_time);


HAMIGAKI_FILESYSTEM_DECL error_code
create_hard_link_api(const std::string& to_ph, const std::string& from_ph);

HAMIGAKI_FILESYSTEM_DECL error_code
create_file_symlink_api(const std::string& to_ph, const std::string& from_ph);

HAMIGAKI_FILESYSTEM_DECL error_code
create_directory_symlink_api(
    const std::string& to_ph, const std::string& from_ph);

HAMIGAKI_FILESYSTEM_DECL error_code
create_symlink_api(const std::string& to_ph, const std::string& from_ph);

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

#if defined(BOOST_WINDOWS)
HAMIGAKI_FILESYSTEM_DECL error_code
create_shell_link_api(
    const std::string& to_ph, const std::string& from_ph,
    const shell_link_options& options);
#endif // defined(BOOST_WINDOWS)

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


HAMIGAKI_FS_FUNC(bool) exists(const Path& ph)
{
    error_code ec;
    const file_status& s =
        detail::status_api(ph.external_file_string(), ec);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::exists", ph, ec);
    }
    return exists(s);
}

HAMIGAKI_FS_FUNC(bool) is_directory(const Path& ph)
{
    error_code ec;
    const file_status& s =
        detail::status_api(ph.external_file_string(), ec);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::is_directory", ph, ec);
    }
    return is_directory(s);
}

HAMIGAKI_FS_FUNC(bool) is_regular(const Path& ph)
{
    error_code ec;
    const file_status& s =
        detail::status_api(ph.external_file_string(), ec);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::is_regular", ph, ec);
    }
    return is_regular(s);
}

HAMIGAKI_FS_FUNC(bool) is_other(const Path& ph)
{
    error_code ec;
    const file_status& s =
        detail::status_api(ph.external_file_string(), ec);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::is_other", ph, ec);
    }
    return is_other(s);
}

HAMIGAKI_FS_FUNC(bool) is_symlink(const Path& ph)
{
    error_code ec;
    const file_status& s =
        detail::symlink_status_api(ph.external_file_string(), ec);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::is_symlink", ph, ec);
    }
    return is_symlink(s);
}


HAMIGAKI_FS_FUNC(Path)
symlink_target(const Path& ph)
{
    HAMIGAKI_FS_TYPENAME Path::external_string_type buf;
    error_code ec = detail::symlink_target_api(ph.external_file_string(), buf);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::symlink_target", ph, ec);
    }
    return Path(Path::traits_type::to_internal(buf));
}

HAMIGAKI_FS_FUNC(void)
last_write_time(const Path& ph, const timestamp& new_time)
{
    error_code ec =
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
    error_code ec =
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
    error_code ec =
        detail::creation_time_api(ph.external_file_string(), new_time);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::creation_time", ph, ec);
    }
}


HAMIGAKI_FS_FUNC(void)
create_hard_link(const Path& to_ph, const Path& from_ph)
{
    error_code ec = detail::create_hard_link_api(
        to_ph.external_file_string(), from_ph.external_file_string());
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::create_hard_link", to_ph, from_ph, ec);
    }
}

HAMIGAKI_FS_FUNC(error_code)
create_hard_link(const Path& to_ph, const Path& from_ph, error_code& ec)
{
    ec = detail::create_hard_link_api(
        to_ph.external_file_string(), from_ph.external_file_string());
    return ec;
}

HAMIGAKI_FS_FUNC(void)
create_file_symlink(const Path& to_ph, const Path& from_ph)
{
    error_code ec = detail::create_file_symlink_api(
        to_ph.external_file_string(), from_ph.external_file_string());
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::create_file_symlink", to_ph, from_ph, ec);
    }
}

HAMIGAKI_FS_FUNC(error_code)
create_file_symlink(const Path& to_ph, const Path& from_ph, error_code& ec)
{
    ec = detail::create_file_symlink_api(
        to_ph.external_file_string(), from_ph.external_file_string());
    return ec;
}

HAMIGAKI_FS_FUNC(void)
create_directory_symlink(const Path& to_ph, const Path& from_ph)
{
    error_code ec = detail::create_directory_symlink_api(
        to_ph.external_directory_string(), from_ph.external_directory_string());
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::create_directory_symlink",
            to_ph, from_ph, ec);
    }
}

HAMIGAKI_FS_FUNC(error_code)
create_directory_symlink(const Path& to_ph, const Path& from_ph, error_code& ec)
{
    ec = detail::create_directory_symlink_api(
        to_ph.external_directory_string(), from_ph.external_directory_string());
    return ec;
}

HAMIGAKI_FS_FUNC(void) create_symlink(const Path& to_ph, const Path& from_ph)
{
    error_code ec = detail::create_symlink_api(
        to_ph.external_file_string(), from_ph.external_file_string());
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::create_symlink", to_ph, from_ph, ec);
    }
}

HAMIGAKI_FS_FUNC(error_code)
create_symlink(const Path& to_ph, const Path& from_ph, error_code& ec)
{
    ec = detail::create_symlink_api(
        to_ph.external_file_string(), from_ph.external_file_string());
    return ec;
}

HAMIGAKI_FS_FUNC(void)
change_attributes(const Path& ph, file_attributes::value_type attr)
{
    error_code ec =
        detail::change_attributes_api(ph.external_file_string(), attr);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::change_attributes", ph, ec);
    }
}

HAMIGAKI_FS_FUNC(error_code)
change_attributes(
    const Path& ph, file_attributes::value_type attr, error_code& ec)
{
    ec = detail::change_attributes_api(ph.external_file_string(), attr);
    return ec;
}

HAMIGAKI_FS_FUNC(void)
change_permissions(const Path& ph, file_permissions::value_type perm)
{
    error_code ec =
        detail::change_permissions_api(ph.external_file_string(), perm);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::change_permissions", ph, ec);
    }
}

HAMIGAKI_FS_FUNC(error_code)
change_permissions(
    const Path& ph, file_permissions::value_type perm, error_code& ec)
{
    ec = detail::change_permissions_api(ph.external_file_string(), perm);
    return ec;
}

HAMIGAKI_FS_FUNC(void)
change_owner(
    const Path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    error_code ec =
        detail::change_owner_api(ph.external_file_string(), new_uid, new_gid);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::change_owner", ph, ec);
    }
}

HAMIGAKI_FS_FUNC(error_code)
change_owner(
    const Path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, error_code& ec)
{
    ec = detail::change_owner_api(ph.external_file_string(), new_uid, new_gid);
    return ec;
}

HAMIGAKI_FS_FUNC(void)
change_symlink_owner(
    const Path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    error_code ec =
        detail::change_symlink_owner_api(
            ph.external_file_string(), new_uid, new_gid
        );
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::change_symlink_owner", ph, ec);
    }
}

HAMIGAKI_FS_FUNC(error_code)
change_symlink_owner(
    const Path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, error_code& ec)
{
    ec = detail::change_symlink_owner_api(
        ph.external_file_string(), new_uid, new_gid);
    return ec;
}

HAMIGAKI_FS_FUNC(unsigned long)
remove_all(const Path& p)
{
    unsigned long n = 0;

    const file_status& s = filesystem::symlink_status(p);

    if (!is_symlink(s) && is_directory(s))
    {
        boost::filesystem::basic_directory_iterator<Path> it(p);
        boost::filesystem::basic_directory_iterator<Path> end;

        for ( ; it != end; ++it)
            n += hamigaki::filesystem::remove_all HAMIGAKI_FS_SPEC (*it);
    }

    if (exists(s))
    {
        remove(p);
        ++n;
    }
    return n;
}

#if defined(BOOST_WINDOWS)
HAMIGAKI_FS_FUNC(void) create_shell_link(
    const Path& to_ph, const Path& from_ph,
    const basic_shell_link_options<
        HAMIGAKI_FS_TYPENAME Path::external_string_type
    >& options)
{
    error_code ec = detail::create_shell_link_api(
        to_ph.external_file_string(), from_ph.external_file_string(), options);
    if (ec)
    {
        throw boost::filesystem::basic_filesystem_error<Path>(
            "hamigaki::filesystem::create_shell_link", to_ph, from_ph, ec);
    }
}

HAMIGAKI_FS_FUNC(error_code)
create_shell_link(
    const Path& to_ph, const Path& from_ph,
    const basic_shell_link_options<
        HAMIGAKI_FS_TYPENAME Path::external_string_type
    >& options, error_code& ec)
{
    ec = detail::create_shell_link_api(
        to_ph.external_file_string(), from_ph.external_file_string(), options);
    return ec;
}

HAMIGAKI_FS_FUNC(void) create_shell_link(const Path& to_ph, const Path& from_ph)
{
    basic_shell_link_options<
        HAMIGAKI_FS_TYPENAME Path::external_string_type
    > options;
    filesystem::create_shell_link(to_ph, from_ph, options);
}

HAMIGAKI_FS_FUNC(error_code)
create_shell_link(const Path& to_ph, const Path& from_ph, error_code& ec)
{
    basic_shell_link_options<
        HAMIGAKI_FS_TYPENAME Path::external_string_type
    > options;
    return filesystem::create_shell_link(to_ph, from_ph, options, ec);
}
#endif // defined(BOOST_WINDOWS)

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
    return hamigaki::filesystem::exists<path>(ph);
}
inline bool exists(const wpath& ph)
{
    return hamigaki::filesystem::exists<wpath>(ph);
}

inline bool is_directory(const path& ph)
{
    return hamigaki::filesystem::is_directory<path>(ph);
}
inline bool is_directory(const wpath& ph)
{
    return hamigaki::filesystem::is_directory<wpath>(ph);
}

inline bool is_regular(const path& ph)
{
    return hamigaki::filesystem::is_regular<path>(ph);
}
inline bool is_regular(const wpath& ph)
{
    return hamigaki::filesystem::is_regular<wpath>(ph);
}

inline bool is_other(const path& ph)
{
    return hamigaki::filesystem::is_other<path>(ph);
}
inline bool is_other(const wpath& ph)
{
    return hamigaki::filesystem::is_other<wpath>(ph);
}

inline bool is_symlink(const path& ph)
{
    return hamigaki::filesystem::is_symlink<path>(ph);
}
inline bool is_symlink(const wpath& ph)
{
    return hamigaki::filesystem::is_symlink<wpath>(ph);
}


inline path symlink_target(const path& ph)
{
    return hamigaki::filesystem::symlink_target<path>(ph);
}
inline wpath symlink_target(const wpath& ph)
{
    return hamigaki::filesystem::symlink_target<wpath>(ph);
}

inline void last_write_time(const path& ph, const timestamp& new_time)
{
    hamigaki::filesystem::last_write_time<path>(ph, new_time);
}
inline void last_write_time(const wpath& ph, const timestamp& new_time)
{
    hamigaki::filesystem::last_write_time<wpath>(ph, new_time);
}

inline void last_access_time(const path& ph, const timestamp& new_time)
{
    hamigaki::filesystem::last_write_time<path>(ph, new_time);
}
inline void last_access_time(const wpath& ph, const timestamp& new_time)
{
    hamigaki::filesystem::last_write_time<wpath>(ph, new_time);
}

inline void creation_time(const path& ph, const timestamp& new_time)
{
    hamigaki::filesystem::creation_time<path>(ph, new_time);
}
inline void creation_time(const wpath& ph, const timestamp& new_time)
{
    hamigaki::filesystem::creation_time<wpath>(ph, new_time);
}


inline void create_hard_link(const path& to_ph, const path& from_ph)
{
    hamigaki::filesystem::create_hard_link<path>(to_ph, from_ph);
}
inline void create_hard_link(const wpath& to_ph, const wpath& from_ph)
{
    hamigaki::filesystem::create_hard_link<wpath>(to_ph, from_ph);
}

inline error_code
create_hard_link(const path& to_ph, const path& from_ph, error_code& ec)
{
    return hamigaki::filesystem::create_hard_link<path>(to_ph, from_ph, ec);
}
inline error_code
create_hard_link(const wpath& to_ph, const wpath& from_ph, error_code& ec)
{
    return hamigaki::filesystem::create_hard_link<wpath>(to_ph, from_ph, ec);
}

inline void create_file_symlink(const path& to_ph, const path& from_ph)
{
    hamigaki::filesystem::create_file_symlink<path>(to_ph, from_ph);
}
inline void create_file_symlink(const wpath& to_ph, const wpath& from_ph)
{
    hamigaki::filesystem::create_file_symlink<wpath>(to_ph, from_ph);
}

inline error_code
create_file_symlink(const path& to_ph, const path& from_ph, error_code& ec)
{
    return hamigaki::filesystem::create_file_symlink<path>(to_ph, from_ph, ec);
}
inline error_code
create_file_symlink(const wpath& to_ph, const wpath& from_ph, error_code& ec)
{
    return hamigaki::filesystem::create_file_symlink<wpath>(to_ph, from_ph, ec);
}

inline void create_directory_symlink(const path& to_ph, const path& from_ph)
{
    hamigaki::filesystem::create_directory_symlink<path>(to_ph, from_ph);
}
inline void create_directory_symlink(const wpath& to_ph, const wpath& from_ph)
{
    hamigaki::filesystem::create_directory_symlink<wpath>(to_ph, from_ph);
}

inline error_code
create_directory_symlink(const path& to_ph, const path& from_ph, error_code& ec)
{
    return hamigaki::filesystem::
        create_directory_symlink<path>(to_ph, from_ph, ec);
}
inline error_code create_directory_symlink(
    const wpath& to_ph, const wpath& from_ph, error_code& ec)
{
    return hamigaki::filesystem::
        create_directory_symlink<wpath>(to_ph, from_ph, ec);
}

inline void create_symlink(const path& to_ph, const path& from_ph)
{
    hamigaki::filesystem::create_symlink<path>(to_ph, from_ph);
}
inline void create_symlink(const wpath& to_ph, const wpath& from_ph)
{
    hamigaki::filesystem::create_symlink<wpath>(to_ph, from_ph);
}

inline error_code
create_symlink(const path& to_ph, const path& from_ph, error_code& ec)
{
    return hamigaki::filesystem::create_symlink<path>(to_ph, from_ph, ec);
}
inline error_code
create_symlink(const wpath& to_ph, const wpath& from_ph, error_code& ec)
{
    return hamigaki::filesystem::create_symlink<wpath>(to_ph, from_ph, ec);
}

inline void change_attributes(const path& ph, file_attributes::value_type attr)
{
    hamigaki::filesystem::change_attributes<path>(ph, attr);
}
inline void change_attributes(const wpath& ph, file_attributes::value_type attr)
{
    hamigaki::filesystem::change_attributes<wpath>(ph, attr);
}

inline error_code change_attributes(
    const path& ph, file_attributes::value_type attr, error_code& ec)
{
    return hamigaki::filesystem::change_attributes<path>(ph, attr, ec);
}
inline error_code change_attributes(
    const wpath& ph, file_attributes::value_type attr, error_code& ec)
{
    return hamigaki::filesystem::change_attributes<wpath>(ph, attr, ec);
}

inline void change_permissions(
    const path& ph, file_permissions::value_type perm)
{
    hamigaki::filesystem::change_permissions<path>(ph, perm);
}
inline void change_permissions(
    const wpath& ph, file_permissions::value_type perm)
{
    hamigaki::filesystem::change_permissions<wpath>(ph, perm);
}

inline error_code change_permissions(
    const path& ph, file_permissions::value_type perm, error_code& ec)
{
    return hamigaki::filesystem::change_permissions<path>(ph, perm, ec);
}
inline error_code change_permissions(
    const wpath& ph, file_permissions::value_type perm, error_code& ec)
{
    return hamigaki::filesystem::change_permissions<wpath>(ph, perm, ec);
}

inline void change_owner(
    const path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    hamigaki::filesystem::change_owner<path>(ph, new_uid, new_gid);
}
inline void change_owner(
    const wpath& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    hamigaki::filesystem::change_owner<wpath>(ph, new_uid, new_gid);
}

inline error_code change_owner(
    const path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, error_code& ec)
{
    return hamigaki::filesystem::change_owner<path>(ph, new_uid, new_gid, ec);
}
inline error_code change_owner(
    const wpath& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, error_code& ec)
{
    return hamigaki::filesystem::change_owner<wpath>(ph, new_uid, new_gid, ec);
}

inline void change_symlink_owner(
    const path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    hamigaki::filesystem::change_symlink_owner<path>(ph, new_uid, new_gid);
}
inline void change_symlink_owner(
    const wpath& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    hamigaki::filesystem::change_symlink_owner<wpath>(ph, new_uid, new_gid);
}

inline error_code change_symlink_owner(
    const path& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, error_code& ec)
{
    return hamigaki::filesystem::
        change_symlink_owner<path>(ph, new_uid, new_gid, ec);
}
inline error_code change_symlink_owner(
    const wpath& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, error_code& ec)
{
    return hamigaki::filesystem::
        change_symlink_owner<wpath>(ph, new_uid, new_gid, ec);
}

inline unsigned long remove_all(const path& p)
{
    return hamigaki::filesystem::remove_all<path>(p);
}
inline unsigned long remove_all(const wpath& p)
{
    return hamigaki::filesystem::remove_all<wpath>(p);
}

#if defined(BOOST_WINDOWS)
inline void create_shell_link(
    const path& to_ph, const path& from_ph, const shell_link_options& options)
{
    hamigaki::filesystem::create_shell_link<path>(to_ph, from_ph, options);
}
inline void create_shell_link(
    const wpath& to_ph, const wpath& from_ph,
    const wshell_link_options& options)
{
    hamigaki::filesystem::create_shell_link<wpath>(to_ph, from_ph, options);
}

inline error_code create_shell_link(
    const path& to_ph, const path& from_ph,
    const shell_link_options& options, error_code& ec)
{
    return hamigaki::filesystem::
        create_shell_link<path>(to_ph, from_ph, options, ec);
}
inline error_code create_shell_link(
    const wpath& to_ph, const wpath& from_ph,
    const wshell_link_options& options, error_code& ec)
{
    return hamigaki::filesystem::
        create_shell_link<wpath>(to_ph, from_ph, options, ec);
}

inline void create_shell_link(const path& to_ph, const path& from_ph)
{
    hamigaki::filesystem::create_shell_link<path>(to_ph, from_ph);
}
inline void create_shell_link(const wpath& to_ph, const wpath& from_ph)
{
    hamigaki::filesystem::create_shell_link<wpath>(to_ph, from_ph);
}

inline error_code
create_shell_link(const path& to_ph, const path& from_ph, error_code& ec)
{
    return hamigaki::filesystem::create_shell_link<path>(to_ph, from_ph, ec);
}
inline error_code
create_shell_link(const wpath& to_ph, const wpath& from_ph, error_code& ec)
{
    return hamigaki::filesystem::create_shell_link<wpath>(to_ph, from_ph, ec);
}
#endif // defined(BOOST_WINDOWS)

#endif // defined(BOOST_FILESYSTEM_NARROW_ONLY)

} } // End namespaces filesystem, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_FILESYSTEM_OPERATIONS_HPP
