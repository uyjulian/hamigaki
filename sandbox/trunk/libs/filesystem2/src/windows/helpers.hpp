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
#include <boost/scoped_array.hpp>
#include <windows.h>

#if (_WIN32_WINNT >= 0x0500)
    #define HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME
    #define HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT
    #include <winioctl.h>
#endif

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

#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
const DWORD mount_point_tag = 0xA0000003;

struct reparse_data_header
{
    DWORD tag;
    WORD length;
    WORD reserved;
};

struct mount_point_header
{
    WORD sub_name_offset;
    WORD sub_name_length;
    WORD print_name_offset;
    WORD print_name_length;
};

inline error_code set_mount_point(HANDLE handle,
    const wchar_t* sub_name, std::size_t sub_name_length,
    const wchar_t* print_name, std::size_t print_name_length)
{
    std::size_t full_size =
        sizeof(reparse_data_header) + sizeof(mount_point_header) +
        (sub_name_length+1 + print_name_length+1) * sizeof(wchar_t);

    boost::scoped_array<char> buf(new char[full_size]);
    char* buf_ptr = buf.get();

    reparse_data_header top_head;
    top_head.tag = mount_point_tag;
    top_head.length = full_size - sizeof(top_head);
    top_head.reserved = 0;
    std::memcpy(buf_ptr, &top_head, sizeof(top_head));
    buf_ptr += sizeof(top_head);

    mount_point_header mt_head;
    mt_head.sub_name_offset = 0;
    mt_head.sub_name_length = sub_name_length * sizeof(wchar_t);
    mt_head.print_name_offset = mt_head.sub_name_length + sizeof(wchar_t);
    mt_head.print_name_length = print_name_length * sizeof(wchar_t);
    std::memcpy(buf_ptr, &mt_head, sizeof(mt_head));
    buf_ptr += sizeof(mt_head);

    std::memcpy(buf_ptr, sub_name, sub_name_length * sizeof(wchar_t));
    buf_ptr += sub_name_length * sizeof(wchar_t);
    std::memset(buf_ptr, 0, sizeof(wchar_t));
    buf_ptr += sizeof(wchar_t);

    std::memcpy(buf_ptr, print_name, print_name_length * sizeof(wchar_t));
    buf_ptr += print_name_length * sizeof(wchar_t);
    std::memset(buf_ptr, 0, sizeof(wchar_t));

    DWORD dummy = 0;
    if (::DeviceIoControl(handle, FSCTL_SET_REPARSE_POINT,
        buf.get(), full_size, 0, 0, &dummy, 0) == 0)
    {
        return make_error_code(static_cast<int>(::GetLastError()));
    }

    return error_code();
}
#endif // defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
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

inline BOOL create_directory(const wchar_t* ph)
{
    return ::CreateDirectoryW(ph, 0);
}

inline BOOL remove_directory(const wchar_t* ph)
{
    return ::RemoveDirectoryW(ph);
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

#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
inline error_code create_symbolic_link(
    const std::wstring& from_ph, const std::wstring& to_ph, DWORD flags)
{
    typedef BOOL (APIENTRY * func_type)(LPCWSTR, LPCWSTR, DWORD);

    const wchar_t* from_s = from_ph.c_str();
    const wchar_t* to_s = to_ph.c_str();

    error_code ec(ERROR_NOT_SUPPORTED, boost::system::get_system_category());
    HMODULE dll(::LoadLibraryA("kernel32.dll"));
    func_type func = reinterpret_cast<func_type>(
        ::GetProcAddress(dll, "CreateSymbolicLinkW"));
    if (func)
    {
        BOOL res = (*func)(from_s, to_s, flags);
        if (res)
            ec = error_code();
        else
            ec = make_error_code(static_cast<int>(::GetLastError()));
    }
    ::FreeLibrary(dll);
    return ec;
}

inline error_code set_mount_point(HANDLE handle, const std::wstring& ph)
{
    std::wstring sub_name(L"\\\?\?\\");
    if ((ph[0] == L'\\') && (ph[1] == L'\\'))
        sub_name += L"UNC";
    sub_name += ph;

    return set_mount_point(
        handle, sub_name.c_str(), sub_name.size(), ph.c_str(), ph.size());
}
#endif // defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)

#endif // !defined(BOOST_FILESYSTEM_NARROW_ONLY)

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

inline BOOL create_directory(const char* ph)
{
    return ::CreateDirectoryA(ph, 0);
}

inline BOOL remove_directory(const char* ph)
{
    return ::RemoveDirectoryA(ph);
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

#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
inline error_code create_symbolic_link(
    const std::string& from_ph, const std::string& to_ph, DWORD flags)
{
    typedef BOOL (APIENTRY * func_type)(LPCSTR, LPCSTR, DWORD);

    const char* from_s = from_ph.c_str();
    const char* to_s = to_ph.c_str();

    error_code ec(ERROR_NOT_SUPPORTED, boost::system::get_system_category());
    HMODULE dll(::LoadLibraryA("kernel32.dll"));
    func_type func = reinterpret_cast<func_type>(
        ::GetProcAddress(dll, "CreateSymbolicLinkA"));
    if (func)
    {
        BOOL res = (*func)(from_s, to_s, flags);
        if (res)
            ec = error_code();
        else
            ec = make_error_code(static_cast<int>(::GetLastError()));
    }
    ::FreeLibrary(dll);
    return ec;
}

inline error_code set_mount_point(HANDLE handle, const std::string & ph)
{
    int w_size = ::MultiByteToWideChar(CP_ACP, 0, ph.c_str(), ph.size(), 0, 0);
    if (w_size == 0)
        return make_error_code(static_cast<int>(::GetLastError()));

    bool is_unc = (ph[0] == '\\') && (ph[1] == '\\');

    std::size_t prefix_length = is_unc ? 7 : 4;
    std::size_t sub_name_length = prefix_length + w_size;

    boost::scoped_array<wchar_t> sub_name(new wchar_t[sub_name_length]);

    wchar_t * ptr = sub_name.get();
    std::memcpy(ptr, L"\\\?\?\\", 4*sizeof(wchar_t));
    ptr += 4;
    if (is_unc)
    {
        std::memcpy(ptr, L"UNC", 3*sizeof(wchar_t));
        ptr += 3;
    }

    w_size =
        ::MultiByteToWideChar(CP_ACP, 0, ph.c_str(), ph.size(), ptr, w_size);
    if (w_size == 0)
        return make_error_code(static_cast<int>(::GetLastError()));

    return set_mount_point(
        handle, &sub_name[0], sub_name_length, ptr, w_size);
}
#endif // defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)


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
class remove_directory_monitor
{
public:
    explicit remove_directory_monitor(const String & ph)
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
inline error_code
create_directory_symlink_template(const String& to_ph, const String& from_ph)
{
#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    if (!create_directory(from_ph.c_str()))
        return make_error_code(static_cast<int>(::GetLastError()));

    remove_directory_monitor<String> guard(from_ph);

    object_handle f(create_file(
        from_ph.c_str(), GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, 0)
    );

    error_code ec = set_mount_point(f.get(), to_ph);
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
    {
        ec = create_symbolic_link(from_ph, to_ph, 0);
        if (ec && (ec == make_error_code(ERROR_NOT_SUPPORTED)))
            return create_directory_symlink_template(to_ph, from_ph);
        else
            return ec;
    }
    else
        return create_symbolic_link(from_ph, to_ph, 0);
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
        return make_error_code(static_cast<int>(::GetLastError()));
}

} } } // End namespaces detail, filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_WINDOWS_HELPERS_HPP
