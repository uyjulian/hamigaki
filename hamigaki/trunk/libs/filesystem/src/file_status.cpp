//  file_status.cpp: the file status operations

//  Copyright Takeshi Mouri 2006, 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#define HAMIGAKI_FILESYSTEM_SOURCE
#define NOMINMAX

#if !defined(BOOST_ALL_NO_LIB)
    #define BOOST_ALL_NO_LIB
#endif

#if !defined(_WIN32_WINNT)
    #define _WIN32_WINNT 0x0500
#endif

#include <boost/config.hpp>

#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/none.hpp>

#if defined(BOOST_WINDOWS)
    #include <windows.h>

    #if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
        #include <hamigaki/detail/windows/dynamic_link_library.hpp>
        #define HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME
    #endif

    // from winbase.h
    #if !defined(INVALID_FILE_ATTRIBUTES)
        #define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
    #endif
#else
    #include <sys/stat.h>
    #include <errno.h>
    #include <utime.h>
#endif

namespace hamigaki { namespace filesystem {

#if defined(BOOST_WINDOWS)

#if defined(HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME)
using namespace ::hamigaki::detail::windows;

// from winternl.h
struct io_status_block
{
    union
    {
        long status;
        void* pointer;
    };

    ::ULONG_PTR information;
};

// from ZwQueryInformationFile in ntddk.h
typedef long (__stdcall *NtQueryInformationFileFuncPtr)(
    ::HANDLE, io_status_block*, void*, ::ULONG, ::DWORD);

// from ntddk.h
struct file_basic_information
{
    static const ::DWORD id = 4;

    ::FILETIME creation_time;
    ::FILETIME last_access_time;
    ::FILETIME last_write_time;
    ::FILETIME last_change_time;
    ::ULONG file_attributes;
    ::DWORD reserved;
};

#endif

inline timestamp to_timestamp(const ::FILETIME& ft)
{
    boost::uint64_t n =
        (static_cast<boost::uint64_t>(ft.dwLowDateTime)       ) |
        (static_cast<boost::uint64_t>(ft.dwHighDateTime) << 32) ;

    return timestamp::from_windows_file_time(n);
}

inline ::FILETIME to_file_time(const timestamp& ts)
{
    boost::uint64_t n = ts.to_windows_file_time();

    ::FILETIME tmp;
    tmp.dwLowDateTime  = static_cast<boost::uint32_t>((n      ));
    tmp.dwHighDateTime = static_cast<boost::uint32_t>((n >> 32));

    return tmp;
}

inline ::FILETIME* to_file_time(
    ::FILETIME& buf, const boost::optional<timestamp>& ts)
{
    if (ts)
    {
        buf = to_file_time(*ts);
        return &buf;
    }
    else
        return static_cast< ::FILETIME*>(0);
}

namespace
{

class file : private boost::noncopyable
{
public:
    file(
        const boost::filesystem::path& p, ::DWORD access, ::DWORD flags)
    {
        handle_ = ::CreateFileA(
            p.native_file_string().c_str(), access,
            FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, flags, 0);
    }

    ~file()
    {
        if (handle_ != INVALID_HANDLE_VALUE)
            ::CloseHandle(handle_);
    }

    bool get_file_information(::BY_HANDLE_FILE_INFORMATION& data)
    {
        if (handle_ == INVALID_HANDLE_VALUE)
            return false;

        return ::GetFileInformationByHandle(handle_, &data) != 0;
    }

    bool set_file_time(
        const boost::optional<timestamp>& creation_time,
        const boost::optional<timestamp>& last_access_time,
        const boost::optional<timestamp>& last_write_time)
    {
        if (handle_ == INVALID_HANDLE_VALUE)
            return false;

        ::FILETIME buf[3];
        ::BOOL res = ::SetFileTime(
            handle_,
            to_file_time(buf[0], creation_time),
            to_file_time(buf[1], last_access_time),
            to_file_time(buf[2], last_write_time)
        );
        return res != FALSE;
    }

#if defined(HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME)
    bool get_last_change_time(timestamp& ts)
    {
        if (handle_ == INVALID_HANDLE_VALUE)
            return false;

        try
        {
            dynamic_link_library ntdll("ntdll.dll");

            NtQueryInformationFileFuncPtr func_ptr =
                reinterpret_cast<NtQueryInformationFileFuncPtr>(
                    ntdll.get_proc_address("NtQueryInformationFile"));

            io_status_block state;
            file_basic_information info;

            long res = (*func_ptr)(handle_, &state,
                &info, sizeof(info), file_basic_information::id);

            if (res != 0)
                return false;

            ts = to_timestamp(info.last_change_time);
            return true;
        }
        catch (const std::exception&)
        {
        }
        return false;
    }
#endif // defined(HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME)

private:
    ::HANDLE handle_;
};

} // namespace

HAMIGAKI_FILESYSTEM_DECL
file_status status(const boost::filesystem::path& p, int& ec)
{
    file_status ss = symlink_status(p, ec);
    if (ss.type() != symlink_file)
        return ss;

    // Win9X doesn't support the reparse points.
    // So, it is only NT that reaches here.

    file f(p, 0, FILE_FLAG_BACKUP_SEMANTICS);

    ::BY_HANDLE_FILE_INFORMATION data;
    if (!f.get_file_information(data))
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
        timestamp ts;
        if (f.get_last_change_time(ts))
            s.last_change_time(ts);
    }
#endif

    return s;
}

HAMIGAKI_FILESYSTEM_DECL
file_status symlink_status(const boost::filesystem::path& p, int& ec)
{
    ec = 0;

    ::WIN32_FILE_ATTRIBUTE_DATA data;
    ::BOOL res = ::GetFileAttributesExA(
        p.native_file_string().c_str(), ::GetFileExInfoStandard, &data);
    if (res == FALSE)
    {
        ::DWORD code = ::GetLastError();
        if ((code == ERROR_FILE_NOT_FOUND) ||
            (code == ERROR_PATH_NOT_FOUND) ||
            (code == ERROR_INVALID_NAME) ||
            (code == ERROR_INVALID_PARAMETER) ||
            (code == ERROR_BAD_NETPATH) )
        {
            return file_status(file_not_found);
        }
        else
        {
            ec = static_cast<int>(code);
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
        file f(p, 0, FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT);
        timestamp ts;
        if (f.get_last_change_time(ts))
            s.last_change_time(ts);
    }
#endif

    return s;
}

HAMIGAKI_FILESYSTEM_DECL
void last_write_time(
    const boost::filesystem::path& p, const timestamp& new_time)
{
    const std::string& name = p.native_file_string();

    ::DWORD attr = ::GetFileAttributesA(name.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES)
    {
        ::DWORD flags = 0;
        if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
            flags |= FILE_FLAG_BACKUP_SEMANTICS;
        if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
            flags |= FILE_FLAG_OPEN_REPARSE_POINT;

        file f(name.c_str(), FILE_WRITE_ATTRIBUTES, flags);
        if (f.set_file_time(boost::none, boost::none, new_time))
            return;

        if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
            return;
    }

    ::DWORD code = ::GetLastError();
    throw hamigaki::filesystem::filesystem_path_error(
        "hamigaki::filesystem::last_write_time", p, code);
}

HAMIGAKI_FILESYSTEM_DECL
void last_access_time(
    const boost::filesystem::path& p, const timestamp& new_time)
{
    const std::string& name = p.native_file_string();

    ::DWORD attr = ::GetFileAttributesA(name.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES)
    {
        ::DWORD flags = 0;
        if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
            flags |= FILE_FLAG_BACKUP_SEMANTICS;
        if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
            flags |= FILE_FLAG_OPEN_REPARSE_POINT;

        file f(name.c_str(), FILE_WRITE_ATTRIBUTES, flags);
        if (f.set_file_time(boost::none, new_time, boost::none))
            return;

        if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
            return;
    }

    ::DWORD code = ::GetLastError();
    throw hamigaki::filesystem::filesystem_path_error(
        "hamigaki::filesystem::last_access_time", p, code);
}

HAMIGAKI_FILESYSTEM_DECL
void creation_time(
    const boost::filesystem::path& p, const timestamp& new_time)
{
    const std::string& name = p.native_file_string();

    ::DWORD attr = ::GetFileAttributesA(name.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES)
    {
        ::DWORD flags = 0;
        if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
            flags |= FILE_FLAG_BACKUP_SEMANTICS;
        if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
            flags |= FILE_FLAG_OPEN_REPARSE_POINT;

        file f(name.c_str(), FILE_WRITE_ATTRIBUTES, flags);
        if (f.set_file_time(new_time, boost::none, boost::none))
            return;

        if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
            return;
    }

    ::DWORD code = ::GetLastError();
    throw hamigaki::filesystem::filesystem_path_error(
        "hamigaki::filesystem::creation_time", p, code);
}

HAMIGAKI_FILESYSTEM_DECL
int change_attributes(
    const boost::filesystem::path& p, file_attributes::value_type attr, int& ec)
{
    ec = 0;
    ::DWORD value = attr;

    if (::SetFileAttributesA(p.native_file_string().c_str(), value) == FALSE)
        ec = ::GetLastError();
    return ec;
}

HAMIGAKI_FILESYSTEM_DECL
int change_permissions(
    const boost::filesystem::path&, file_permissions::value_type, int& ec)
{
    ec = ERROR_NOT_SUPPORTED;
    return ec;
}

HAMIGAKI_FILESYSTEM_DECL
int change_owner(
    const boost::filesystem::path&,
    const boost::optional<boost::intmax_t>&,
    const boost::optional<boost::intmax_t>&, int& ec)
{
    ec = ERROR_NOT_SUPPORTED;
    return ec;
}

HAMIGAKI_FILESYSTEM_DECL
int change_symlink_owner(
    const boost::filesystem::path&,
    const boost::optional<boost::intmax_t>&,
    const boost::optional<boost::intmax_t>&, int& ec)
{
    ec = ERROR_NOT_SUPPORTED;
    return ec;
}

#else // not defined(BOOST_WINDOWS)

inline file_status make_file_status(
    const boost::filesystem::path& p, struct stat& data)
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

HAMIGAKI_FILESYSTEM_DECL
file_status status(const boost::filesystem::path& p, int& ec)
{
    ec = 0;

    struct stat data;
    if (::stat(p.native_file_string().c_str(), &data) == -1)
    {
        int code = errno;
        if ((code == ENOENT) || (code == ENOTDIR))
            return file_status(file_not_found);
        else
        {
            ec = code;
            return file_status(status_unknown);
        }
    }

    return filesystem::make_file_status(p, data);
}

HAMIGAKI_FILESYSTEM_DECL
file_status symlink_status(const boost::filesystem::path& p, int& ec)
{
    ec = 0;

    struct stat data;
    if (::lstat(p.native_file_string().c_str(), &data) == -1)
    {
        int code = errno;
        if ((code == ENOENT) || (code == ENOTDIR))
            return file_status(file_not_found);
        else
        {
            ec = code;
            return file_status(status_unknown);
        }
    }

    return filesystem::make_file_status(p, data);
}

HAMIGAKI_FILESYSTEM_DECL
void last_write_time(
    const boost::filesystem::path& p, const timestamp& new_time)
{
    const std::string& name = p.native_file_string();

    struct stat st;
    if (::stat(name.c_str(), &st) != -1)
    {
        ::utimbuf buf;
        buf.actime = st.st_atime;
        buf.modtime = new_time.to_time_t();
        if (::utime(name.c_str(), &buf) != -1)
            return;
    }

    int code = errno;
    throw hamigaki::filesystem::filesystem_path_error(
        "hamigaki::filesystem::last_write_time", p, code);
}

HAMIGAKI_FILESYSTEM_DECL
void last_access_time(
    const boost::filesystem::path& p, const timestamp& new_time)
{
    const std::string& name = p.native_file_string();

    struct stat st;
    if (::stat(name.c_str(), &st) != -1)
    {
        ::utimbuf buf;
        buf.actime = new_time.to_time_t();
        buf.modtime = st.st_mtime;
        if (::utime(name.c_str(), &buf) != -1)
            return;
    }

    int code = errno;
    throw hamigaki::filesystem::filesystem_path_error(
        "hamigaki::filesystem::last_access_time", p, code);
}

HAMIGAKI_FILESYSTEM_DECL
void creation_time(const boost::filesystem::path&, const timestamp&)
{
    // do nothing
}

HAMIGAKI_FILESYSTEM_DECL
int change_attributes(
    const boost::filesystem::path& p, file_attributes::value_type attr, int& ec)
{
    ec = ENOTSUP;
    return ec;
}

HAMIGAKI_FILESYSTEM_DECL
int change_permissions(
    const boost::filesystem::path& p,
    file_permissions::value_type perm, int& ec)
{
    ec = 0;
    ::mode_t mode = perm;

    if (::chmod(p.native_file_string().c_str(), mode) == -1)
        ec = errno;
    return ec;
}

HAMIGAKI_FILESYSTEM_DECL
int change_owner(
    const boost::filesystem::path& p,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, int& ec)
{
    ec = 0;

    int res = ::chown(
        p.native_file_string().c_str(),
        new_uid ? static_cast< ::uid_t>(*new_uid) : static_cast< ::uid_t>(-1),
        new_gid ? static_cast< ::gid_t>(*new_gid) : static_cast< ::gid_t>(-1)
    );

    if (res == -1)
        ec = errno;

    return ec;
}

HAMIGAKI_FILESYSTEM_DECL
int change_symlink_owner(
    const boost::filesystem::path& p,
    const boost::optional<boost::intmax_t>& new_uid,
    const boost::optional<boost::intmax_t>& new_gid, int& ec)
{
    ec = 0;

    int res = ::lchown(
        p.native_file_string().c_str(),
        new_uid ? static_cast< ::uid_t>(*new_uid) : static_cast< ::uid_t>(-1),
        new_gid ? static_cast< ::gid_t>(*new_gid) : static_cast< ::gid_t>(-1)
    );

    if (res == -1)
        ec = errno;

    return ec;
}

#endif

} } // End namespaces filesystem, hamigaki.
