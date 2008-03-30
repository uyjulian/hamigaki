// windows_error.hpp: Windows error class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/system for library home page.

#ifndef HAMIGAKI_SYSTEM_WINDOWS_ERROR_HPP
#define HAMIGAKI_SYSTEM_WINDOWS_ERROR_HPP

#include <hamigaki/system/system_error.hpp>
#include <stdarg.h>

extern "C"
{

__declspec(dllimport)
unsigned long __stdcall FormatMessageA(
    unsigned long dwFlags,
    const void* lpSource,
    unsigned long dwMessageId,
    unsigned long dwLanguageId,
    char* lpBuffer,
    unsigned long nSize,
    va_list* Arguments
);

__declspec(dllimport)
void* __stdcall LocalFree(void* hMem);

} // extern "C"

namespace hamigaki { namespace system {

struct windows_error_traits
{
    typedef unsigned long value_type;

    static std::string message(unsigned long code)
    {
        std::string tmp;

        char* ptr;
        unsigned long len = ::FormatMessageA(
            0x00001300, // ALLOCATE_BUFFER, IGNORE_INSERTS, FROM_SYSTEM
            0, code,
            0x400, // MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
            reinterpret_cast<char*>(&ptr), 0, 0
        );
        if (len == 0)
            return tmp;

        char* end = ptr + len;
        if (end[-1] == '\n')
        {
            --end;
            if ((end != ptr) && end[-1] == '\r')
                --end;
        }

        try
        {
            tmp.assign(ptr, end);
        }
        catch (...)
        {
            ::LocalFree(ptr);
            throw;
        }
        ::LocalFree(ptr);
        return tmp;
    }
};

typedef system_error<windows_error_traits> windows_error;

} } // End namespaces system, hamigaki.

#endif // HAMIGAKI_SYSTEM_WINDOWS_ERROR_HPP
