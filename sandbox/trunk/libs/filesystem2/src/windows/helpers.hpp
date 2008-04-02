// helpers.hpp: the helper functions for Windows

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_WINDOWS_HELPERS_HPP
#define HAMIGAKI_FILESYSTEM_WINDOWS_HELPERS_HPP

#include <hamigaki/filesystem2/detail/config.hpp>
#include <hamigaki/filesystem2/file_status.hpp>
#include <windows.h>

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

#ifndef BOOST_FILESYSTEM_NARROW_ONLY
inline HANDLE create_file(
    const wchar_t* ph, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    return ::CreateFileW(
        ph, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
        dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile
    );
}

inline BOOL get_file_attributes_ex(
    const wchar_t* ph,
    GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
    return ::GetFileAttributesExW(ph, fInfoLevelId, lpFileInformation);
}

inline DWORD get_file_attributes(const wchar_t* ph)
{
    return ::GetFileAttributesW(ph);
}

inline BOOL set_file_attributes(const wchar_t* ph, DWORD dwFileAttributes)
{
    return ::SetFileAttributesW(ph, dwFileAttributes);
}
#endif

#if defined(HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME)
// from winternl.h
struct io_status_block
{
    union
    {
        LONG status;
        void* pointer;
    };

    ULONG_PTR information;
};

// from ntddk.h
struct file_basic_information
{
    static const DWORD id = 4;

    FILETIME creation_time;
    FILETIME last_access_time;
    FILETIME last_write_time;
    FILETIME last_change_time;
    ULONG file_attributes;
    DWORD reserved;
};

inline bool query_information_file(
    HANDLE FileHandle, io_status_block* IoStatusBlock,
    void* FileInformation, ULONG Length, DWORD FileInformationClass)
{
    // from ZwQueryInformationFile in ntddk.h
    typedef LONG (__stdcall *func_type)(
        HANDLE, io_status_block*, void*, ULONG, DWORD);

    bool result = false;

    HMODULE dll(::LoadLibraryA("ntdll.dll"));
    if (!dll)
        return result;

    func_type func = reinterpret_cast<func_type>(
        ::GetProcAddress(dll, "NtQueryInformationFile"));
    if (func)
    {
        LONG res = (*func)(
            FileHandle, IoStatusBlock, FileInformation, Length,
            FileInformationClass);

        if (res == 0)
            result = true;
    }
    ::FreeLibrary(dll);

    return result;
}
#endif // defined(HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME)

inline HANDLE create_file(
    const char* ph, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    return ::CreateFileA(
        ph, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
        dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

inline BOOL get_file_attributes_ex(
    const char* ph,
    GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
    return ::GetFileAttributesExA(ph, fInfoLevelId, lpFileInformation);
}

inline DWORD get_file_attributes(const char* ph)
{
    return ::GetFileAttributesA(ph);
}

inline BOOL set_file_attributes(const char* ph, DWORD dwFileAttributes)
{
    return ::SetFileAttributesA(ph, dwFileAttributes);
}


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
last_write_time_template(const String& ph, const timestamp& new_time)
{
    DWORD attr = get_file_attributes(ph.c_str());

    if (attr == INVALID_FILE_ATTRIBUTES)
        return make_error_code(static_cast<int>(::GetLastError()));

    DWORD flags = 0;
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        flags |= FILE_FLAG_BACKUP_SEMANTICS;
    if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
        flags |= FILE_FLAG_OPEN_REPARSE_POINT;

    object_handle f(create_file(
        ph.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE,
        0, OPEN_EXISTING, flags, 0)
    );

    FILETIME ft = to_file_time(new_time);
    if (::SetFileTime(f.get(), 0, 0, &ft))
        return error_code();

    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return error_code();
    else
        return make_error_code(static_cast<int>(::GetLastError()));
}

template<class String>
inline error_code
last_access_time_template(const String& ph, const timestamp& new_time)
{
    DWORD attr = get_file_attributes(ph.c_str());

    if (attr == INVALID_FILE_ATTRIBUTES)
        return make_error_code(static_cast<int>(::GetLastError()));

    DWORD flags = 0;
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        flags |= FILE_FLAG_BACKUP_SEMANTICS;
    if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
        flags |= FILE_FLAG_OPEN_REPARSE_POINT;

    object_handle f(create_file(
        ph.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE,
        0, OPEN_EXISTING, flags, 0)
    );

    FILETIME ft = to_file_time(new_time);
    if (::SetFileTime(f.get(), 0, &ft, 0))
        return error_code();

    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return error_code();
    else
        return make_error_code(static_cast<int>(::GetLastError()));
}

template<class String>
inline error_code
creation_time_template(const String& ph, const timestamp& new_time)
{
    DWORD attr = get_file_attributes(ph.c_str());

    if (attr == INVALID_FILE_ATTRIBUTES)
        return make_error_code(static_cast<int>(::GetLastError()));

    DWORD flags = 0;
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        flags |= FILE_FLAG_BACKUP_SEMANTICS;
    if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
        flags |= FILE_FLAG_OPEN_REPARSE_POINT;

    object_handle f(create_file(
        ph.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE,
        0, OPEN_EXISTING, flags, 0)
    );

    FILETIME ft = to_file_time(new_time);
    if (::SetFileTime(f.get(), &ft, 0, 0))
        return error_code();

    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return error_code();
    else
        return make_error_code(static_cast<int>(::GetLastError()));
}

template<class String>
inline error_code
change_attributes_template(const String& ph, file_attributes::value_type attr)
{
    if (set_file_attributes(ph.c_str(), static_cast<DWORD>(attr)))
        return error_code();
    else
        return make_error_code(static_cast<int>(::GetLastError()));
}

} } } // End namespaces detail, filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_WINDOWS_HELPERS_HPP
