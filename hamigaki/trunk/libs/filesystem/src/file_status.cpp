//  file_status.cpp: the file status operations

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#define HAMIGAKI_FILESYSTEM_SOURCE
#define BOOST_ALL_NO_LIB
#define NOMINMAX

#if !defined(_WIN32_WINNT)
    #define _WIN32_WINNT 0x0500
#endif

#include <boost/config.hpp>

#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>

#if defined(BOOST_WINDOWS)
    #include <windows.h>

    #if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
        #include <hamigaki/detail/windows/dynamic_link_library.hpp>
        #define HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME
    #endif
#else
    #include <sys/stat.h>
    #include <errno.h>
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

template<class Data>
inline file_attributes make_attr(const Data& data)
{
    file_attributes attr = 0;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
        attr |= read_only;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0)
        attr |= hidden;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0)
        attr |= system;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0)
        attr |= archive;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) != 0)
        attr |= sparse;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0)
        attr |= temporary;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0)
        attr |= compressed;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) != 0)
        attr |= offline;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) != 0)
        attr |= not_indexed;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) != 0)
        attr |= encrypted;
    return attr;
}

inline bool is_executable_path(const boost::filesystem::path& p)
{
    const std::string& ext = boost::filesystem::extension(p);

    return (
        (ext == ".exe") || (ext == ".com") ||
        (ext == ".bat") || (ext == ".cmd") );
}

namespace
{

class nt_file : private boost::noncopyable
{
public:
    explicit nt_file(const boost::filesystem::path& p, ::DWORD flags=0)
    {
        handle_ = ::CreateFileA(
            p.native_file_string().c_str(), 0,
            FILE_SHARE_READ|FILE_SHARE_WRITE, 0,
            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS|flags, 0);
    }

    ~nt_file()
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

    nt_file f(p);

    ::BY_HANDLE_FILE_INFORMATION data;
    if (!f.get_file_information(data))
        return file_status(status_unknown);

    BOOST_ASSERT((data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0);

    file_type type = regular_file;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        type = directory_file;
    else if ((data.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) != 0)
        type = type_unknown;

    file_attributes attr = make_attr(data);

    file_permissions perm = 0600;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
        perm &= ~0200;

    if (is_executable_path(p) || (type == directory_file))
        perm |= 0100;

    file_status s(type);
    s.attributes(attr);
    s.permissions(perm);

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

    file_attributes attr = make_attr(data);

    file_permissions perm = 0600;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
        perm &= ~0200;

    if (is_executable_path(p) || (type == directory_file))
        perm |= 0100;

    file_status s(type);
    s.attributes(attr);
    s.permissions(perm);

    s.file_size(
        (static_cast<boost::uint64_t>(data.nFileSizeHigh) << 32) |
        (static_cast<boost::uint64_t>(data.nFileSizeLow)       ) );

    s.last_write_time(to_timestamp(data.ftLastWriteTime));
    s.last_access_time(to_timestamp(data.ftLastAccessTime));
    s.creation_time(to_timestamp(data.ftCreationTime));

#if defined(HAMIGAKI_FILESYSTEM_USE_NTFS_CHANGE_TIME)
    {
        nt_file f(p, FILE_FLAG_OPEN_REPARSE_POINT);
        timestamp ts;
        if (f.get_last_change_time(ts))
            s.last_change_time(ts);
    }
#endif

    return s;
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

    file_attributes attr = 0;
    if ((data.st_mode & S_ISUID) != 0)
        attr |= set_uid;
    if ((data.st_mode & S_ISGID) != 0)
        attr |= set_gid;
    if ((data.st_mode & S_ISVTX) != 0)
        attr |= sticky;

    if ((data.st_mode & 0222) == 0)
        attr |= read_only;
    if (p.has_leaf() && (p.leaf()[0] == '.'))
        attr |= hidden;

    file_status s(type);
    s.attributes(attr);
    s.permissions(data.st_mode & 0777);
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

#endif

} } // End namespaces filesystem, hamigaki.
