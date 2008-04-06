// file_status.ipp: the file status operations for POSIX

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include <hamigaki/filesystem2/detail/config.hpp>
#include <sys/stat.h>
#include <errno.h>
#include <utime.h>

namespace hamigaki { namespace filesystem { namespace detail {

inline file_status make_file_status(struct stat& data)
{
    file_type type = type_unknown;
    if (S_ISREG(data.st_mode))
        type = regular_file;
    else if (S_ISDIR(data.st_mode))
        type = directory_file;
    else if (S_ISLNK(data.st_mode))
        type = symlink_file;
    else if (S_ISBLK(data.st_mode))
        type = block_file;
    else if (S_ISCHR(data.st_mode))
        type = character_file;
    else if (S_ISFIFO(data.st_mode))
        type = fifo_file;
    else if (S_ISSOCK(data.st_mode))
        type = socket_file;

    file_status s(type);
    s.permissions(data.st_mode);
    s.file_size(data.st_size);
    s.last_write_time(timestamp::from_time_t(data.st_mtime));
    s.last_access_time(timestamp::from_time_t(data.st_atime));
    s.last_change_time(timestamp::from_time_t(data.st_ctime));
    s.uid(data.st_uid);
    s.gid(data.st_gid);

    return s;
}

HAMIGAKI_FILESYSTEM_DECL file_status
status_api(const std::string& ph, error_code& ec)
{
    struct stat data;
    if (::stat(ph.c_str(), &data) == -1)
    {
        int code = errno;
        if ((code == ENOENT) || (code == ENOTDIR))
        {
            ec = error_code();
            return file_status(file_not_found);
        }
        else
        {
            ec = make_error_code(code);
            return file_status(status_unknown);
        }
    }

    ec = error_code();
    return detail::make_file_status(data);
}

HAMIGAKI_FILESYSTEM_DECL file_status
symlink_status_api(const std::string& ph, error_code& ec)
{
    struct stat data;
    if (::lstat(ph.c_str(), &data) == -1)
    {
        int code = errno;
        if ((code == ENOENT) || (code == ENOTDIR))
        {
            ec = error_code();
            return file_status(file_not_found);
        }
        else
        {
            ec = make_error_code(code);
            return file_status(status_unknown);
        }
    }

    ec = error_code();
    return detail::make_file_status(data);
}

HAMIGAKI_FILESYSTEM_DECL error_code
last_write_time_api(const std::string& ph, const timestamp& new_time)
{
    struct stat st;
    if (::stat(ph.c_str(), &st) == -1)
        return make_error_code(errno);

    utimbuf buf;
    buf.actime = st.st_atime;
    buf.modtime = new_time.to_time_t();
    if (::utime(ph.c_str(), &buf) == -1)
        return make_error_code(errno);

    return error_code();
}

HAMIGAKI_FILESYSTEM_DECL error_code
last_access_time_api(const std::string& ph, const timestamp& new_time)
{
    struct stat st;
    if (::stat(ph.c_str(), &st) == -1)
        return make_error_code(errno);

    utimbuf buf;
    buf.actime = new_time.to_time_t();
    buf.modtime = st.st_mtime;
    if (::utime(ph.c_str(), &buf) == -1)
        return make_error_code(errno);

    return error_code();
}

HAMIGAKI_FILESYSTEM_DECL error_code
creation_time_api(const std::string& ph, const timestamp& new_time)
{
    return error_code();
}


HAMIGAKI_FILESYSTEM_DECL error_code
change_attributes_api(const std::string&, file_attributes::value_type)
{
    return make_error_code(ENOTSUP);
}

HAMIGAKI_FILESYSTEM_DECL
error_code change_permissions_api(
    const std::string& ph, file_permissions::value_type perm)
{
    mode_t mode = static_cast<mode_t>(perm);

    if (::chmod(ph.c_str(), mode) == -1)
        return make_error_code(errno);

    return error_code();
}

HAMIGAKI_FILESYSTEM_DECL
error_code change_owner_api(
    const std::string& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    int res = ::chown(
        ph.c_str(),
        new_uid ? static_cast<uid_t>(*new_uid) : static_cast<uid_t>(-1),
        new_gid ? static_cast<gid_t>(*new_gid) : static_cast<gid_t>(-1)
    );

    if (res == -1)
        return make_error_code(errno);

    return error_code();
}

HAMIGAKI_FILESYSTEM_DECL
error_code change_symlink_owner_api(
    const std::string& ph,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid)
{
    int res = ::lchown(
        ph.c_str(),
        new_uid ? static_cast<uid_t>(*new_uid) : static_cast<uid_t>(-1),
        new_gid ? static_cast<gid_t>(*new_gid) : static_cast<gid_t>(-1)
    );

    if (res == -1)
        return make_error_code(errno);

    return error_code();
}

} } } // End namespaces detail, filesystem, hamigaki.
