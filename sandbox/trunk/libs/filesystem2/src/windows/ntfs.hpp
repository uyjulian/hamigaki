// ntfs.hpp: the functions for the NTFS

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_WINDOWS_NTFS_HPP
#define HAMIGAKI_FILESYSTEM_WINDOWS_NTFS_HPP

#include <windows.h>

namespace hamigaki { namespace filesystem { namespace detail {

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

} } } // End namespaces detail, filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_WINDOWS_NTFS_HPP
