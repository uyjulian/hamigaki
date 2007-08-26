// symlink.cpp: the symbolic link operations

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

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
#include <boost/filesystem/operations.hpp>
#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>
#include <cstring>
#include <vector>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/binary/binary_io.hpp>
    #include <windows.h>

    #include <hamigaki/detail/windows/dynamic_link_library.hpp>

    #if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
        #include "detail/reparse_point.hpp"
        #include <winioctl.h>
        #define HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT
    #endif

    // from winbase.h
    #if !defined(INVALID_FILE_ATTRIBUTES)
        #define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
    #endif
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #include <errno.h>
#endif

namespace hamigaki { namespace filesystem {

#if defined(BOOST_WINDOWS)

namespace
{

#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
// from winbase.h
struct symlink_flags
{
    static const ::DWORD directory = 0x00000001;
};

// from winbase.h
typedef ::BOOL (__stdcall *CreateSymbolicLinkWFuncPtr)(
    const wchar_t*, const wchar_t*, ::DWORD);

inline std::string to_multi_byte(const wchar_t* s, int n)
{
    int size = ::WideCharToMultiByte(CP_ACP, 0, s, n, 0, 0, 0, 0);
    if (size == 0)
        return std::string();

    std::vector<char> buf(size);
    size = ::WideCharToMultiByte(CP_ACP, 0, s, n, &buf[0], size, 0, 0);
    if (size == 0)
        return std::string();
    buf.resize(size);

    return std::string(&buf[0], size);
}

inline std::wstring to_wide(const char* s, int n)
{
    int size = ::MultiByteToWideChar(CP_ACP, 0, s, n, 0, 0);
    if (size == 0)
        return std::wstring();

    std::vector<wchar_t> buf(size);
    size = ::MultiByteToWideChar(CP_ACP, 0, s, n, &buf[0], size);
    if (size == 0)
        return std::wstring();
    buf.resize(size);

    return std::wstring(&buf[0], size);
}

inline std::wstring to_wide(const std::string& s)
{
    return to_wide(s.c_str(), static_cast<int>(s.size()));
}

class nt_file : private boost::noncopyable
{
public:
    nt_file(const boost::filesystem::path& p, ::DWORD mode, ::DWORD flags)
        : ec_(0)
    {
        handle_ = ::CreateFileA(
            p.native_file_string().c_str(), mode,
            FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS|flags, 0);

        if (!is_valid())
            ec_ = ::GetLastError();
    }

    ~nt_file()
    {
        if (is_valid())
            ::CloseHandle(handle_);
    }

    bool is_valid() const
    {
        return handle_ != INVALID_HANDLE_VALUE;
    }

    int native_error() const
    {
        return static_cast<int>(ec_);
    }

    bool get_reparse_point(boost::filesystem::path& p)
    {
        BOOST_ASSERT(is_valid());

        std::vector<char> buf(16*1024);

        ::DWORD size = 0;
        if (::DeviceIoControl(handle_, FSCTL_GET_REPARSE_POINT,
            0, 0, &buf[0], buf.size(), &size, 0) == 0)
        {
            ec_ = ::GetLastError();
            return false;
        }

        const std::size_t hedaer_size =
            struct_size<detail::reparse_data_header>::value;

        if (static_cast<std::size_t>(size) < hedaer_size)
            return false;

        detail::reparse_data_header head;
        hamigaki::binary_read(&buf[0], head);

        const char* data = &buf[0] + hedaer_size;
        std::size_t data_size = size - hedaer_size;

        if (static_cast<std::size_t>(head.length) != data_size)
            return false;

        if (head.tag == detail::mount_point_header::tag)
        {
            const std::size_t hedaer_size =
                struct_size<detail::mount_point_header>::value;

            if (data_size < hedaer_size)
                return false;

            detail::mount_point_header head;
            hamigaki::binary_read(data, head);

            data += hedaer_size;
            data_size -= hedaer_size;

            p = parse_absolute_path(head, data, data_size);
        }
        else if (head.tag == detail::symlink_header::tag)
        {
            const std::size_t hedaer_size =
                struct_size<detail::symlink_header>::value;

            if (data_size < hedaer_size)
                return false;

            detail::symlink_header head;
            hamigaki::binary_read(data, head);

            data += hedaer_size;
            data_size -= hedaer_size;

            if ((head.flags & detail::symlink_flags::relative) != 0)
                p = parse_relative_path(head, data, data_size);
            else
                p = parse_absolute_path(head, data, data_size);
        }
        else
            return false;

        return !p.empty();
    }

    bool set_reparse_point(const boost::filesystem::path& p)
    {
        BOOST_ASSERT(is_valid());

        std::wstring sub_name(L"\\\?\?\\");
        const std::wstring& ws =
            to_wide(system_complete(p).native_file_string());
        if ((ws.size() > 2) && (ws[0] == L'\\') && (ws[1] == L'\\'))
            sub_name += L"UNC\\";
        sub_name += ws;

        const std::size_t top_hedaer_size =
            struct_size<detail::reparse_data_header>::value;
        const std::size_t mt_hedaer_size =
            struct_size<detail::mount_point_header>::value;

        std::size_t full_size =
            top_hedaer_size + mt_hedaer_size +
            (sub_name.size()+2) * sizeof(wchar_t);

        std::vector<char> buf(full_size);

        detail::reparse_data_header top_head;
        top_head.tag = detail::mount_point_header::tag;
        top_head.length = full_size - top_hedaer_size;
        top_head.reserved = 0;
        hamigaki::binary_write(&buf[0], top_head);

        detail::mount_point_header mt_head;
        mt_head.sub_name_offset = 0;
        mt_head.sub_name_length = sub_name.size() * sizeof(wchar_t);
        mt_head.print_name_offset = mt_head.sub_name_length + sizeof(wchar_t);
        mt_head.print_name_length = 0;
        hamigaki::binary_write(&buf[top_hedaer_size], mt_head);

        std::memcpy(
            &buf[top_hedaer_size+mt_hedaer_size],
            sub_name.c_str(),
            sub_name.size() * sizeof(wchar_t)
        );

        ::DWORD dummy = 0;
        if (::DeviceIoControl(handle_, FSCTL_SET_REPARSE_POINT,
            &buf[0], buf.size(), 0, 0, &dummy, 0) == 0)
        {
            ec_ = ::GetLastError();
            return false;
        }

        return true;
    }

private:
    ::HANDLE handle_;
    ::DWORD ec_;

    template<class Header>
    boost::filesystem::path parse_absolute_path(
        const Header& head, const char* data, std::size_t data_size)
    {
        const wchar_t prefix[] = L"\\\?\?\\";
        std::size_t prefix_size = sizeof(prefix)/sizeof(wchar_t) - 1;

        std::size_t total = static_cast<std::size_t>(
            head.sub_name_offset + head.sub_name_length);

        if ((head.sub_name_length < prefix_size) ||
            (head.sub_name_offset > data_size) ||
            (total > data_size) )
        {
            return boost::filesystem::path();
        }

        std::vector<wchar_t> sub_name(head.sub_name_length/sizeof(wchar_t));
        std::memcpy(&sub_name[0],
            data+head.sub_name_offset, head.sub_name_length);
        if (std::wmemcmp(&sub_name[0], prefix, prefix_size) != 0)
            return boost::filesystem::path();

        wchar_t* src = &sub_name[prefix_size];
        std::size_t src_size = sub_name.size() - prefix_size;

        const wchar_t unc[] = L"UNC\\";
        std::size_t unc_size = sizeof(unc)/sizeof(wchar_t) - 1;

        if ((src_size > unc_size) && (std::wmemcmp(src, unc, unc_size) == 0))
        {
            src += (unc_size-2);
            *src = '\\';
            src_size -= (unc_size-2);
        }

        return boost::filesystem::path(
            to_multi_byte(src, static_cast<int>(src_size)),
            boost::filesystem::no_check
        );
    }

    boost::filesystem::path parse_relative_path(
        const detail::symlink_header& head,
        const char* data, std::size_t data_size)
    {
        std::size_t total = static_cast<std::size_t>(
            head.sub_name_offset + head.sub_name_length);

        if ((head.sub_name_offset > data_size) || (total > data_size))
            return boost::filesystem::path();

        std::vector<wchar_t> sub_name(head.sub_name_length/sizeof(wchar_t));
        std::memcpy(&sub_name[0],
            data+head.sub_name_offset, head.sub_name_length);

        const wchar_t* src = &sub_name[0];
        int src_size = static_cast<int>(sub_name.size());

        return boost::filesystem::path(
            to_multi_byte(src, src_size),
            boost::filesystem::no_check
        );
    }
};

class remove_directory_monitor : private boost::noncopyable
{
public:
    explicit remove_directory_monitor(const char* ph)
        : path_(ph), need_(true)
    {
    }

    ~remove_directory_monitor()
    {
        if (need_)
            ::RemoveDirectoryA(path_);
    }

    void release()
    {
        need_ = false;
    }

private:
    const char* path_;
    bool need_;
};

int create_symlink_impl(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, ::DWORD flags)
{
    int ec = ERROR_NOT_SUPPORTED;
    try
    {
        hamigaki::detail::windows::dynamic_link_library dll("kernel32.dll");

        CreateSymbolicLinkWFuncPtr func_ptr =
            reinterpret_cast<CreateSymbolicLinkWFuncPtr>(
                dll.get_proc_address("CreateSymbolicLinkW"));
        ec = 0;

        ::BOOL res = (*func_ptr)(
            to_wide(new_fp.native_file_string()).c_str(),
            to_wide(old_fp.native_file_string()).c_str(), flags);

        if (res == FALSE)
            return ::GetLastError();
    }
    catch (const std::exception&)
    {
    }
    return ec;
}

#endif // defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)

} // namespace

HAMIGAKI_FILESYSTEM_DECL
boost::filesystem::path symlink_target(const boost::filesystem::path& p)
{
#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    nt_file f(p, 0, FILE_FLAG_OPEN_REPARSE_POINT);
    if (!f.is_valid())
    {
        throw hamigaki::filesystem::filesystem_path_error(
            "hamigaki::filesystem::symlink_target", p,
            make_error_code(f.native_error()));
    }

    boost::filesystem::path tp;
    if (!f.get_reparse_point(tp))
    {
        throw hamigaki::filesystem::filesystem_path_error(
            "hamigaki::filesystem::symlink_target", p,
            make_error_code(f.native_error()));
    }
    return tp;
#else // else defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    throw hamigaki::filesystem::filesystem_path_error(
        "hamigaki::filesystem::symlink_target", p,
        "unsupported operation");

    BOOST_UNREACHABLE_RETURN(boost::filesystem::path())
#endif // else defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
}

HAMIGAKI_FILESYSTEM_DECL
int create_hard_link(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec)
{
    ec = ERROR_NOT_SUPPORTED;
    try
    {
        hamigaki::detail::windows::dynamic_link_library dll("kernel32.dll");

        typedef ::BOOL (__stdcall *CreateHardLinkAFuncPtr)(
            const char*, const char*, void*);

        CreateHardLinkAFuncPtr func_ptr =
            reinterpret_cast<CreateHardLinkAFuncPtr>(
                dll.get_proc_address("CreateHardLinkA"));
        ec = 0;

        ::BOOL res = (*func_ptr)(
            new_fp.native_file_string().c_str(),
            old_fp.native_file_string().c_str(), 0);

        if (res == FALSE)
        {
            ec = ::GetLastError();
            return ec;
        }
    }
    catch (const std::exception&)
    {
    }
    return ec;
}

HAMIGAKI_FILESYSTEM_DECL
int create_file_symlink(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec)
{
#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    ec = create_symlink_impl(old_fp, new_fp, 0);
    return ec;
#else // else defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    ec = ERROR_NOT_SUPPORTED;
    return ec;
#endif // else defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
}

HAMIGAKI_FILESYSTEM_DECL
int create_directory_symlink(
    const boost::filesystem::path& old_dp,
    const boost::filesystem::path& new_dp, int& ec)
{
#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    ec = create_symlink_impl(old_dp, new_dp, symlink_flags::directory);
    if (ec != ERROR_NOT_SUPPORTED)
        return ec;

    ec = 0;

    const std::string& new_name = new_dp.native_directory_string();
    const char* new_str = new_name.c_str();

    if (::CreateDirectoryA(new_str, 0) == FALSE)
    {
        ec = ::GetLastError();
        return ec;
    }
    remove_directory_monitor mon(new_str);

    nt_file file(new_dp, GENERIC_WRITE, 0);
    if (file.is_valid())
    {
        if (file.set_reparse_point(old_dp))
            mon.release();
        else
            ec = file.native_error();
    }
    else
        ec = file.native_error();

    return ec;
#else // else defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    ec = ERROR_NOT_SUPPORTED;
    return ec;
#endif // else defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
}

HAMIGAKI_FILESYSTEM_DECL
int create_symlink(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec)
{
    ::DWORD attr = ::GetFileAttributesA(old_fp.native_file_string().c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        ec = ERROR_PATH_NOT_FOUND;
        return ec;
    }

    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return create_directory_symlink(old_fp, new_fp, ec);
    else
        return create_file_symlink(old_fp, new_fp, ec);
}

#else // not defined(BOOST_WINDOWS)
inline int create_symlink_impl(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec)
{
    ec = 0;

    int res = ::symlink(
        old_fp.native_file_string().c_str(),
        new_fp.native_file_string().c_str());
    if (res == -1)
        ec = errno;

    return ec;
}

HAMIGAKI_FILESYSTEM_DECL
boost::filesystem::path symlink_target(const boost::filesystem::path& p)
{
    const std::string& path_name = p.native_file_string();

    struct stat st;
    if (::lstat(path_name.c_str(), &st) == -1)
    {
        throw hamigaki::filesystem::filesystem_path_error(
            "hamigaki::filesystem::symlink_target", p, make_error_code(errno));
    }

    if (!S_ISLNK(st.st_mode))
    {
        throw hamigaki::filesystem::filesystem_path_error(
            "hamigaki::filesystem::symlink_target "
            "the path is not a symbolic link", p, make_error_code(EINVAL));
    }

    std::vector<char> buf(st.st_size);
    std::streamsize len = ::readlink(path_name.c_str(), &buf[0], buf.size());
    if (len == -1)
    {
        throw hamigaki::filesystem::filesystem_path_error(
            "hamigaki::filesystem::symlink_target", p, make_error_code(errno));
    }
    else if (static_cast<std::size_t>(len) != buf.size())
    {
        // Note: calling readlink() after lstat() has a race condition
        throw hamigaki::filesystem::filesystem_path_error(
            "hamigaki::filesystem::symlink_target "
            "symbolic link size mismatch", p, make_error_code(0));
    }

    return boost::filesystem::path(
        std::string(&buf[0], buf.size()), boost::filesystem::native);
}

HAMIGAKI_FILESYSTEM_DECL
int create_hard_link(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec)
{
    ec = 0;

    int res = ::link(
        old_fp.native_file_string().c_str(),
        new_fp.native_file_string().c_str());
    if (res == -1)
        ec = errno;

    return ec;
}

HAMIGAKI_FILESYSTEM_DECL
int create_file_symlink(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec)
{
    return create_symlink_impl(old_fp, new_fp, ec);
}

HAMIGAKI_FILESYSTEM_DECL
int create_directory_symlink(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec)
{
    return create_symlink_impl(old_fp, new_fp, ec);
}

HAMIGAKI_FILESYSTEM_DECL
int create_symlink(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec)
{
    return create_symlink_impl(old_fp, new_fp, ec);
}
#endif

} } // End namespaces filesystem, hamigaki.
