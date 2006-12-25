//  operations.hpp: the file operations

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_OPERATIONS_HPP
#define HAMIGAKI_FILESYSTEM_OPERATIONS_HPP

#include <hamigaki/filesystem/detail/config.hpp>
#include <hamigaki/filesystem/detail/auto_link.hpp>
#include <hamigaki/filesystem/file_status.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace filesystem {

// status functions

HAMIGAKI_FILESYSTEM_DECL file_status
status(const boost::filesystem::path& p, int& ec);

inline file_status status(const boost::filesystem::path& p)
{
    int ec;
    const file_status& s = filesystem::status(p, ec);
    if (ec != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::status", p, ec);
    }
    return s;
}


HAMIGAKI_FILESYSTEM_DECL file_status
symlink_status(const boost::filesystem::path& p, int& ec);

inline file_status symlink_status(const boost::filesystem::path& p)
{
    int ec;
    const file_status& s = filesystem::symlink_status(p, ec);
    if (ec != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::symlink_status", p, ec);
    }
    return s;
}


// predicate functions

inline bool exists(const boost::filesystem::path& p)
{
    return exists(filesystem::status(p));
}

inline bool is_directory(const boost::filesystem::path& p)
{
    return is_directory(filesystem::status(p));
}

inline bool is_regular(const boost::filesystem::path& p)
{
    return is_regular(filesystem::status(p));
}

inline bool is_other(const boost::filesystem::path& p)
{
    return is_other(filesystem::status(p));
}

inline bool is_symlink(const boost::filesystem::path& p)
{
    return is_symlink(filesystem::symlink_status(p));
}


// attribute functions

HAMIGAKI_FILESYSTEM_DECL
boost::filesystem::path symlink_target(const boost::filesystem::path& p);

HAMIGAKI_FILESYSTEM_DECL
void last_write_time(
    const boost::filesystem::path& p, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL
void last_access_time(
    const boost::filesystem::path& p, const timestamp& new_time);

HAMIGAKI_FILESYSTEM_DECL
void creation_time(
    const boost::filesystem::path& p, const timestamp& new_time);


// operations functions
HAMIGAKI_FILESYSTEM_DECL
int create_hard_link(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec);

inline void create_hard_link(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp)
{
    int ec;
    if (filesystem::create_hard_link(old_fp, new_fp, ec) != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::create_hard_link", old_fp, new_fp, ec);
    }
}

HAMIGAKI_FILESYSTEM_DECL
int create_file_symlink(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec);

inline void create_file_symlink(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp)
{
    int ec;
    if (filesystem::create_file_symlink(old_fp, new_fp, ec) != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::create_file_symlink", old_fp, new_fp, ec);
    }
}

HAMIGAKI_FILESYSTEM_DECL
int create_directory_symlink(
    const boost::filesystem::path& old_dp,
    const boost::filesystem::path& new_dp, int& ec);

inline void create_directory_symlink(
    const boost::filesystem::path& old_dp,
    const boost::filesystem::path& new_dp)
{
    int ec;
    if (filesystem::create_directory_symlink(old_dp, new_dp, ec) != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::create_directory_symlink",
            old_dp, new_dp, ec);
    }
}

HAMIGAKI_FILESYSTEM_DECL
int create_symlink(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec);

inline void create_symlink(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp)
{
    int ec;
    if (filesystem::create_symlink(old_fp, new_fp, ec) != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::create_symlink", old_fp, new_fp, ec);
    }
}

HAMIGAKI_FILESYSTEM_DECL
int change_attributes(
    const boost::filesystem::path& p,
    file_attributes::value_type attr, int& ec);

inline void change_attributes(
    const boost::filesystem::path& p, file_attributes::value_type attr)
{
    int ec;
    if (filesystem::change_attributes(p, attr, ec) != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::change_attributes", p, ec);
    }
}

HAMIGAKI_FILESYSTEM_DECL
int change_permissions(
    const boost::filesystem::path& p,
    file_permissions::value_type perm, int& ec);

inline void change_permissions(
    const boost::filesystem::path& p, file_permissions::value_type perm)
{
    int ec;
    if (filesystem::change_permissions(p, perm, ec) != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::change_permissions", p, ec);
    }
}

HAMIGAKI_FILESYSTEM_DECL
int change_owner(
    const boost::filesystem::path& p,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, int& ec);

inline void change_owner(
    const boost::filesystem::path& p,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    int ec;
    if (filesystem::change_owner(p, new_uid, new_gid, ec) != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::change_owner", p, ec);
    }
}

HAMIGAKI_FILESYSTEM_DECL
int change_symlink_owner(
    const boost::filesystem::path& p,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, int& ec);

inline void change_symlink_owner(
    const boost::filesystem::path& p,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    int ec;
    if (filesystem::change_symlink_owner(p, new_uid, new_gid, ec) != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::change_symlink_owner", p, ec);
    }
}

inline unsigned long remove_all(const boost::filesystem::path& p)
{
    unsigned long n = 0;

    const file_status& s = filesystem::symlink_status(p);

    if (!is_symlink(s) && is_directory(s))
    {
        boost::filesystem::directory_iterator it(p);
        boost::filesystem::directory_iterator end;

        for ( ; it != end; ++it)
            n += filesystem::remove_all(*it);
    }

    if (remove(p))
        ++n;

    return n;
}

} } // End namespaces filesystem, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_FILESYSTEM_OPERATIONS_HPP
