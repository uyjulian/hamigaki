// reparse_point.hpp: the functions for the reparse points

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_WINDOWS_REPARSE_POINT_HPP
#define HAMIGAKI_FILESYSTEM_WINDOWS_REPARSE_POINT_HPP

#include <hamigaki/filesystem/detail/config.hpp>
#include <boost/scoped_array.hpp>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <winioctl.h>

#include "./error_code.hpp"

namespace hamigaki { namespace filesystem { namespace detail {

const DWORD mount_point_tag = 0xA0000003;
const DWORD symlink_tag = 0xA000000C;

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

struct symlink_header
{
    WORD sub_name_offset;
    WORD sub_name_length;
    WORD print_name_offset;
    WORD print_name_length;
    DWORD flags;
};

template<class Header>
inline void parse_absolute_path(
    const Header& head, const char* data, std::size_t data_size,
    std::vector<wchar_t>& target)
{
    const wchar_t prefix[] = L"\\\?\?\\";
    std::size_t prefix_size = sizeof(prefix)/sizeof(wchar_t) - 1;

    std::size_t total = static_cast<std::size_t>(
        head.sub_name_offset + head.sub_name_length);

    if ((head.sub_name_length < prefix_size) ||
        (head.sub_name_offset > data_size) ||
        (total > data_size) )
    {
        throw std::runtime_error("bad reparse-data");
    }

    std::vector<wchar_t> buf(head.sub_name_length/sizeof(wchar_t));
    if (buf.empty())
    {
        target.clear();
        return;
    }
    std::memcpy(&buf[0], data+head.sub_name_offset, buf.size()*sizeof(wchar_t));

    if (std::memcmp(&buf[0], prefix, prefix_size*sizeof(wchar_t)) != 0)
        throw std::runtime_error("bad reparse-data");

    wchar_t* src = &buf[prefix_size];
    std::size_t src_size = buf.size() - prefix_size;

    const wchar_t unc[] = L"UNC\\";
    std::size_t unc_size = sizeof(unc)/sizeof(wchar_t) - 1;

    if ((src_size > unc_size) && (std::wmemcmp(src, unc, unc_size) == 0))
    {
        src += (unc_size-2);
        *src = '\\';
        src_size -= (unc_size-2);
    }

    target.assign(src, src+src_size);
}

inline void parse_relative_path(
    const symlink_header& head, const char* data, std::size_t data_size,
    std::vector<wchar_t>& target)
{
    std::size_t total = static_cast<std::size_t>(
        head.sub_name_offset + head.sub_name_length);

    if ((head.sub_name_offset > data_size) || (total > data_size))
        throw std::runtime_error("bad reparse-data");

    std::vector<wchar_t> buf(head.sub_name_length/sizeof(wchar_t));
    if (buf.empty())
    {
        target.clear();
        return;
    }
    std::memcpy(&buf[0], data+head.sub_name_offset, buf.size()*sizeof(wchar_t));
    target.swap(buf);
}

inline void parse_mount_point_data(
    const char* p, std::size_t size, std::vector<wchar_t>& target)
{
    mount_point_header head;

    if (size < sizeof(head))
        throw std::runtime_error("bad reparse-data");

    std::memcpy(&head, p, sizeof(head));

    p += sizeof(head);
    size -= sizeof(head);

    parse_absolute_path(head, p, size, target);
}

inline void parse_symlink_data(
    const char* p, std::size_t size, std::vector<wchar_t>& target)
{
    if (size < sizeof(symlink_header))
        throw std::runtime_error("bad reparse-data");

    symlink_header head;
    std::memcpy(&head, p, sizeof(head));

    p += sizeof(head);
    size -= sizeof(head);

    if ((head.flags & 1ul) != 0)
        parse_relative_path(head, p, size, target);
    else
        parse_absolute_path(head, p, size, target);
}

inline error_code
get_reparse_point(HANDLE handle, std::vector<wchar_t>& target)
{
    const std::size_t buf_size = 16*1024;
    boost::scoped_array<char> buf(new char[buf_size]);

    DWORD size = 0;
    if (::DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT,
        0, 0, &buf[0], buf_size, &size, 0) == 0)
    {
        return last_error();
    }

    reparse_data_header head;
    if (static_cast<std::size_t>(size) < sizeof(head))
        throw std::runtime_error("bad reparse-data");

    const char* p = &buf[0];
    std::memcpy(&head, p, sizeof(head));
    p += sizeof(head);

    const std::size_t data_size =
        static_cast<std::size_t>(size) - sizeof(reparse_data_header);

    if (static_cast<std::size_t>(head.length) != data_size)
        throw std::runtime_error("bad reparse-data");

    if (head.tag == mount_point_tag)
        parse_mount_point_data(p, data_size, target);
    else if (head.tag == symlink_tag)
        parse_symlink_data(p, data_size, target);
    else
        throw std::runtime_error("unsupported reparse-data");

    return error_code();
}

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
    top_head.length = static_cast<WORD>(full_size - sizeof(top_head));
    top_head.reserved = 0;
    std::memcpy(buf_ptr, &top_head, sizeof(top_head));
    buf_ptr += sizeof(top_head);

    mount_point_header mt_head;
    mt_head.sub_name_offset = 0;
    mt_head.sub_name_length =
        static_cast<WORD>(sub_name_length * sizeof(wchar_t));
    mt_head.print_name_offset =
        static_cast<WORD>(mt_head.sub_name_length + sizeof(wchar_t));
    mt_head.print_name_length =
        static_cast<WORD>(print_name_length * sizeof(wchar_t));
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
        buf.get(), static_cast<DWORD>(full_size), 0, 0, &dummy, 0) == 0)
    {
        return last_error();
    }

    return error_code();
}

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
inline error_code create_symbolic_link(
    const std::wstring& from_ph, const std::wstring& to_ph, DWORD flags)
{
    typedef BOOL (APIENTRY *func_type)(LPCWSTR, LPCWSTR, DWORD);

    const wchar_t* from_s = from_ph.c_str();
    const wchar_t* to_s = to_ph.c_str();

    error_code ec = make_error_code(ERROR_NOT_SUPPORTED);
    HMODULE dll(::LoadLibraryA("kernel32.dll"));
    func_type func = reinterpret_cast<func_type>(
        ::GetProcAddress(dll, "CreateSymbolicLinkW"));
    if (func)
    {
        BOOL res = (*func)(from_s, to_s, flags);
        if (res)
            ec = error_code();
        else
            ec = last_error();
    }
    ::FreeLibrary(dll);
    return ec;
}

inline error_code get_reparse_point(HANDLE handle, std::wstring& target)
{
    std::vector<wchar_t> buf;
    error_code ec = get_reparse_point(handle, buf);
    if (!ec)
        target.assign(buf.begin(), buf.end());
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
#endif // defined(BOOST_FILESYSTEM_NARROW_ONLY)

inline error_code create_symbolic_link(
    const std::string& from_ph, const std::string& to_ph, DWORD flags)
{
    typedef BOOL (APIENTRY *func_type)(LPCSTR, LPCSTR, DWORD);

    const char* from_s = from_ph.c_str();
    const char* to_s = to_ph.c_str();

    error_code ec = make_error_code(ERROR_NOT_SUPPORTED);
    HMODULE dll(::LoadLibraryA("kernel32.dll"));
    func_type func = reinterpret_cast<func_type>(
        ::GetProcAddress(dll, "CreateSymbolicLinkA"));
    if (func)
    {
        BOOL res = (*func)(from_s, to_s, flags);
        if (res)
            ec = error_code();
        else
            ec = last_error();
    }
    ::FreeLibrary(dll);
    return ec;
}

inline error_code get_reparse_point(HANDLE handle, std::string& target)
{
    std::vector<wchar_t> wbuf;
    error_code ec = get_reparse_point(handle, wbuf);
    if (!ec)
    {
        if (wbuf.empty())
        {
            target.clear();
            return error_code();
        }

        int size = ::WideCharToMultiByte(
            CP_ACP, 0, &wbuf[0], static_cast<int>(wbuf.size()), 0, 0, 0, 0);
        if (size == 0)
            return last_error();

        boost::scoped_array<char> buf(new char[size]);
        size = ::WideCharToMultiByte(
            CP_ACP, 0, &wbuf[0], static_cast<int>(wbuf.size()),
            buf.get(), size, 0, 0);
        if (size == 0)
            return last_error();
        target.assign(buf.get(), buf.get()+size);
    }
    return ec;
}

inline error_code set_mount_point(HANDLE handle, const std::string& ph)
{
    int w_size = ::MultiByteToWideChar(
        CP_ACP, 0, ph.c_str(), static_cast<int>(ph.size()), 0, 0);
    if (w_size == 0)
        return last_error();

    bool is_unc = (ph[0] == '\\') && (ph[1] == '\\');

    std::size_t prefix_length = is_unc ? 7 : 4;
    std::size_t sub_name_length = prefix_length + w_size;

    boost::scoped_array<wchar_t> sub_name(new wchar_t[sub_name_length]);

    wchar_t* ptr = sub_name.get();
    std::memcpy(ptr, L"\\\?\?\\", 4*sizeof(wchar_t));
    ptr += 4;
    if (is_unc)
    {
        std::memcpy(ptr, L"UNC", 3*sizeof(wchar_t));
        ptr += 3;
    }

    w_size = ::MultiByteToWideChar(
        CP_ACP, 0, ph.c_str(), static_cast<int>(ph.size()), ptr, w_size);
    if (w_size == 0)
        return last_error();

    return set_mount_point(
        handle, &sub_name[0], sub_name_length, ptr, w_size);
}

} } } // End namespaces detail, filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_WINDOWS_REPARSE_POINT_HPP
