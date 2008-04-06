// helpers.hpp: the helper functions for Windows

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_WINDOWS_HELPERS_HPP
#define HAMIGAKI_FILESYSTEM_WINDOWS_HELPERS_HPP

#include <hamigaki/filesystem/detail/config.hpp>
#include <hamigaki/filesystem/file_status.hpp>
#include <windows.h>

#if (_WIN32_WINNT >= 0x0500)
    #define HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME
    #define HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT
#endif

#if defined(HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME)
    #include "./ntfs.hpp"
#endif

#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    #include "./reparse_point.hpp"
#endif

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    #include "./wide_functions.hpp"
#endif

#include "./narrow_functions.hpp"
#include "./error_code.hpp"

namespace hamigaki { namespace filesystem { namespace detail {

inline timestamp to_timestamp(const FILETIME& ft)
{
    boost::uint64_t n =
        (static_cast<boost::uint64_t>(ft.dwLowDateTime)       ) |
        (static_cast<boost::uint64_t>(ft.dwHighDateTime) << 32) ;

    return timestamp::from_windows_file_time(n);
}

inline FILETIME to_file_time(const timestamp& ts)
{
    boost::uint64_t n = ts.to_windows_file_time();

    FILETIME tmp;
    tmp.dwLowDateTime  = static_cast<boost::uint32_t>((n      ));
    tmp.dwHighDateTime = static_cast<boost::uint32_t>((n >> 32));

    return tmp;
}

class object_handle
{
public:
    explicit object_handle(HANDLE h) : handle_(h)
    {
    }

    ~object_handle()
    {
        if (handle_ != INVALID_HANDLE_VALUE)
            ::CloseHandle(handle_);
    }

    HANDLE get() const
    {
        return handle_;
    }

private:
    HANDLE handle_;

    object_handle(const object_handle&);
    object_handle& operator=(const object_handle&);
};

template<class String>
class remove_directory_monitor
{
public:
    explicit remove_directory_monitor(const String& ph)
        : path_(ph), need_remove_(true)
    {
    }

    ~remove_directory_monitor()
    {
        if (need_remove_)
            detail::remove_directory(path_.c_str());
    }

    void release()
    {
        need_remove_ = false;
    }

private:
    const String& path_;
    bool need_remove_;
};


template<class String>
inline file_status symlink_status_template(const String& ph, error_code& ec)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    BOOL res = get_file_attributes_ex(ph.c_str(), GetFileExInfoStandard, &data);
    if (res == FALSE)
    {
        DWORD code = ::GetLastError();
        if ((code == ERROR_FILE_NOT_FOUND) ||
            (code == ERROR_PATH_NOT_FOUND) ||
            (code == ERROR_INVALID_NAME) ||
            (code == ERROR_INVALID_PARAMETER) ||
            (code == ERROR_BAD_PATHNAME) ||
            (code == ERROR_BAD_NETPATH) )
        {
            ec = error_code();
            return file_status(file_not_found);
        }
        else
        {
            ec = make_error_code(static_cast<int>(code));
            return file_status(status_unknown);
        }
    }

    file_type type = regular_file;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
        type = symlink_file;
    else if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        type = directory_file;
    else if ((data.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) != 0)
        type = type_unknown;

    file_attributes::value_type attr = data.dwFileAttributes;

    file_status s(type);
    s.attributes(attr);

    s.file_size(
        (static_cast<boost::uint64_t>(data.nFileSizeHigh) << 32) |
        (static_cast<boost::uint64_t>(data.nFileSizeLow)       ) );

    s.last_write_time(to_timestamp(data.ftLastWriteTime));
    s.last_access_time(to_timestamp(data.ftLastAccessTime));
    s.creation_time(to_timestamp(data.ftCreationTime));

#if defined(HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME)
    {
        object_handle f(create_file(
            ph.c_str(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE,
            0, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT, 0)
        );

        io_status_block state;
        file_basic_information info;

        if (query_information_file(f.get(), &state,
            &info, sizeof(info), file_basic_information::id))
        {
            s.last_change_time(to_timestamp(info.last_change_time));
        }
    }
#endif

    return s;
}

template<class String>
inline file_status status_template(const String& ph, error_code& ec)
{
    file_status ss = symlink_status_template(ph, ec);
    if (ss.type() != symlink_file)
        return ss;

    object_handle f(create_file(
        ph.c_str(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE,
        0, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, 0)
    );

    BY_HANDLE_FILE_INFORMATION data;
    if (!::GetFileInformationByHandle(f.get(), &data))
        return file_status(status_unknown);

    BOOST_ASSERT((data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0);

    file_type type = regular_file;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        type = directory_file;
    else if ((data.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) != 0)
        type = type_unknown;

    file_attributes::value_type attr = data.dwFileAttributes;

    file_status s(type);
    s.attributes(attr);

    s.file_size(
        (static_cast<boost::uint64_t>(data.nFileSizeHigh) << 32) |
        (static_cast<boost::uint64_t>(data.nFileSizeLow)       ) );

    s.last_write_time(to_timestamp(data.ftLastWriteTime));
    s.last_access_time(to_timestamp(data.ftLastAccessTime));
    s.creation_time(to_timestamp(data.ftCreationTime));

#if defined(HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME)
    {
        io_status_block state;
        file_basic_information info;

        if (query_information_file(f.get(), &state,
            &info, sizeof(info), file_basic_information::id))
        {
            s.last_change_time(to_timestamp(info.last_change_time));
        }
    }
#endif

    return s;
}

template<class String>
inline error_code
set_file_time_template(const String& ph, const FILETIME* lpCreationTime,
    const FILETIME* lpLastAccessTime, const FILETIME* lpLastWriteTime)
{
    DWORD attr = get_file_attributes(ph.c_str());

    if (attr == INVALID_FILE_ATTRIBUTES)
        return last_error();

    DWORD flags = 0;
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        flags |= FILE_FLAG_BACKUP_SEMANTICS;
    if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
        flags |= FILE_FLAG_OPEN_REPARSE_POINT;

    object_handle f(create_file(
        ph.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE,
        0, OPEN_EXISTING, flags, 0)
    );

    if (::SetFileTime(f.get(),lpCreationTime,lpLastAccessTime,lpLastWriteTime))
        return error_code();

    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return error_code();
    else
        return last_error();
}

template<class String>
inline error_code
last_write_time_template(const String& ph, const timestamp& new_time)
{
    FILETIME ft = to_file_time(new_time);
    return detail::set_file_time_template(ph, 0, 0, &ft);
}

template<class String>
inline error_code
last_access_time_template(const String& ph, const timestamp& new_time)
{
    FILETIME ft = to_file_time(new_time);
    return detail::set_file_time_template(ph, 0, &ft, 0);
}

template<class String>
inline error_code
creation_time_template(const String& ph, const timestamp& new_time)
{
    FILETIME ft = to_file_time(new_time);
    return detail::set_file_time_template(ph, &ft, 0, 0);
}

template<class String>
inline error_code
symlink_target_template(const String& ph, String& target)
{
#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    object_handle f(create_file(
        ph.c_str(), 0,
        FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT, 0)
    );

    return detail::get_reparse_point(f.get(), target);
#else
    return make_error_code(ERROR_NOT_SUPPORTED);
#endif
}

template<class String>
inline error_code
create_hard_link_template(const String& to_ph, const String& from_ph)
{
#if (_WIN32_WINNT >= 0x500)
    if (detail::create_hard_link(from_ph.c_str(), to_ph.c_str()))
        return error_code();
    else
        return last_error();
#else
    return make_error_code(ERROR_NOT_SUPPORTED);
#endif
}

template<class String>
inline error_code
create_file_symlink_template(const String& to_ph, const String& from_ph)
{
#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    return detail::create_symbolic_link(from_ph, to_ph, 0);
#else
    return make_error_code(ERROR_NOT_SUPPORTED);
#endif
}

template<class String>
inline error_code
create_directory_symlink_template(const String& to_ph, const String& from_ph)
{
#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    error_code ec = detail::create_symbolic_link(from_ph, to_ph, 1);
    if (!ec || (ec != make_error_code(ERROR_NOT_SUPPORTED)))
        return ec;

    if (!create_directory(from_ph.c_str()))
        return last_error();

    remove_directory_monitor<String> guard(from_ph);

    object_handle f(create_file(
        from_ph.c_str(), GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, 0)
    );

    ec = detail::set_mount_point(f.get(), to_ph);
    if (!ec)
        guard.release();
    return ec;
#else
    return make_error_code(ERROR_NOT_SUPPORTED);
#endif
}

template<class String>
inline error_code
create_symlink_template(const String& to_ph, const String& from_ph)
{
#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    error_code ec;
    file_status s(detail::symlink_status_template(to_ph, ec));
    if (ec)
        return ec;

    if (filesystem::is_directory(s))
        return detail::create_directory_symlink_template(to_ph, from_ph);
    else
        return detail::create_symbolic_link(from_ph, to_ph, 0);
#else
    return make_error_code(ERROR_NOT_SUPPORTED);
#endif
}

template<class String>
inline error_code
change_attributes_template(const String& ph, file_attributes::value_type attr)
{
    if (set_file_attributes(ph.c_str(), static_cast<DWORD>(attr)))
        return error_code();
    else
        return last_error();
}

} } } // End namespaces detail, filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_WINDOWS_HELPERS_HPP
