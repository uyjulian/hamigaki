// wide_functions.hpp: the wide function wrappers

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_WINDOWS_WIDE_FUNCTIONS_HPP
#define HAMIGAKI_FILESYSTEM_WINDOWS_WIDE_FUNCTIONS_HPP

#include <windows.h>

namespace hamigaki { namespace filesystem { namespace detail {

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

#if (_WIN32_WINNT >= 0x500)
inline BOOL create_hard_link(const wchar_t* from_ph, const wchar_t* to_ph)
{
    return ::CreateHardLinkW(from_ph, to_ph, 0);
}
#endif

} } } // End namespaces detail, filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_WINDOWS_WIDE_FUNCTIONS_HPP
