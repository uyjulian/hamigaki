//  symlink.cpp: the symbolic link operations

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#define HAMIGAKI_FILESYSTEM_SOURCE
#define BOOST_ALL_NO_LIB
#define NOMINMAX

#include <boost/config.hpp>

#include <hamigaki/filesystem/operations.hpp>
#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>
#include <cstring>
#include <vector>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/binary_io.hpp>
    #include <windows.h>

    #if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
        #include "detail/reparse_point.hpp"
        #include <winioctl.h>
        #define HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT
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

class nt_file : private boost::noncopyable
{
public:
    explicit nt_file(const boost::filesystem::path& p) : ec_(0)
    {
        handle_ = ::CreateFileA(
            p.native_file_string().c_str(), 0,
            FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT, 0);

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
            binary_size<detail::reparse_data_header>::type::value;

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
                binary_size<detail::mount_point_header>::type::value;

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
                binary_size<detail::symlink_header>::type::value;

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

private:
    ::HANDLE handle_;
    ::DWORD ec_;

    template<class Header>
    boost::filesystem::path parse_absolute_path(
        const Header& head, const char* data, std::size_t data_size)
    {
        const wchar_t prefix[] = L"\\\?\?\\";
        std::size_t prefix_size = sizeof(prefix)/sizeof(wchar_t) - 1;

        if ((head.sub_name_length < prefix_size) ||
            (head.sub_name_offset > data_size) ||
            (head.sub_name_offset + head.sub_name_length > data_size) )
        {
            return boost::filesystem::path();
        }

        std::vector<wchar_t> sub_name(head.sub_name_length/sizeof(wchar_t));
        std::memcpy(&sub_name[0],
            data+head.sub_name_offset, head.sub_name_length);
        if (std::wmemcmp(&sub_name[0], prefix, prefix_size) != 0)
            return boost::filesystem::path();

        wchar_t* src = &sub_name[prefix_size];
        int src_size = static_cast<int>(sub_name.size() - prefix_size);

        const wchar_t unc[] = L"UNC\\";
        std::size_t unc_size = sizeof(unc)/sizeof(wchar_t) - 1;

        if (std::wmemcmp(src, unc, unc_size) == 0)
        {
            src += (unc_size-2);
            *src = '\\';
            src_size -= (unc_size-2);
        }

        return boost::filesystem::path(
            to_multi_byte(src, src_size),
            boost::filesystem::no_check
        );
    }

    boost::filesystem::path parse_relative_path(
        const detail::symlink_header& head,
        const char* data, std::size_t data_size)
    {
        if ((head.sub_name_offset > data_size) ||
            (head.sub_name_offset + head.sub_name_length > data_size) )
        {
            return boost::filesystem::path();
        }

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
#endif // defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)

} // namespace

HAMIGAKI_FILESYSTEM_DECL
boost::filesystem::path symlink_target(const boost::filesystem::path& p)
{
#if defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    nt_file f(p);
    if (!f.is_valid())
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::symlink_target", p, f.native_error());
    }

    boost::filesystem::path tp;
    if (!f.get_reparse_point(tp))
    {
        if (int ec = f.native_error())
        {
            throw boost::filesystem::filesystem_error(
                "hamigaki::filesystem::symlink_target", p, ec);
        }
        else
        {
            throw boost::filesystem::filesystem_error(
                "hamigaki::filesystem::symlink_target", p,
                "invalid reparse point data");
        }
    }
    return tp;
#else // else defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
    throw boost::filesystem::filesystem_error(
        "hamigaki::filesystem::symlink_target", p,
        "unsupported operation");

    BOOST_UNREACHABLE_RETURN(boost::filesystem::path())
#endif // else defined(HAMIGAKI_FILESYSTEM_USE_REPARSE_POINT)
}

#else // not defined(BOOST_WINDOWS)

HAMIGAKI_FILESYSTEM_DECL
boost::filesystem::path symlink_target(const boost::filesystem::path& p)
{
    const std::string& path_name = p.native_file_string();

    struct stat st;
    if (::lstat(path_name.c_str(), &st) == -1)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::symlink_target", p, errno);
    }

    if (!S_ISLNK(st.st_mode))
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::symlink_target", p,
            "the path is not a symbolic link");
    }

    std::vector<char> buf(st.st_size);
    std::streamsize len = ::readlink(path_name.c_str(), &buf[0], buf.size());
    if (len == -1)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::symlink_target", p, errno);
    }
    else if (static_cast<std::size_t>(len) != buf.size())
    {
        // Note: calling readlink() after lstat() has a race condition
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::symlink_target", p,
            "symbolic link size mismatch");
    }

    return boost::filesystem::path(
        std::string(&buf[0], buf.size()), boost::filesystem::native);
}

#endif

} } // End namespaces filesystem, hamigaki.
