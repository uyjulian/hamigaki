// reparse_point.hpp: the functions for the reparse points

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_WINDOWS_REPARSE_POINT_HPP
#define HAMIGAKI_FILESYSTEM_WINDOWS_REPARSE_POINT_HPP

#include <hamigaki/filesystem2/detail/config.hpp>
#include <boost/scoped_array.hpp>
#include <winioctl.h>

#include "./error_code.hpp"

namespace hamigaki { namespace filesystem { namespace detail {

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
            ec = last_error();
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
#endif // defined(BOOST_FILESYSTEM_NARROW_ONLY)

inline error_code create_symbolic_link(
    const std::string& from_ph, const std::string& to_ph, DWORD flags)
{
    typedef BOOL (APIENTRY *func_type)(LPCSTR, LPCSTR, DWORD);

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
            ec = last_error();
    }
    ::FreeLibrary(dll);
    return ec;
}

inline error_code set_mount_point(HANDLE handle, const std::string& ph)
{
    int w_size = ::MultiByteToWideChar(CP_ACP, 0, ph.c_str(), ph.size(), 0, 0);
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

    w_size =
        ::MultiByteToWideChar(CP_ACP, 0, ph.c_str(), ph.size(), ptr, w_size);
    if (w_size == 0)
        return last_error();

    return set_mount_point(
        handle, &sub_name[0], sub_name_length, ptr, w_size);
}

} } } // End namespaces detail, filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_WINDOWS_REPARSE_POINT_HPP
