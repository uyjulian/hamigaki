// windows_error.hpp: Windows error class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/system for library home page.

#ifndef HAMIGAKI_SYSTEM_WINDOWS_ERROR_HPP
#define HAMIGAKI_SYSTEM_WINDOWS_ERROR_HPP

#include <exception>
#include <string>
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

inline std::string get_windows_error_message(unsigned long code)
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

class windows_error : public std::exception
{
public:
    windows_error() : code_(0)
    {
    }

    windows_error(unsigned long code, const char* msg)
        : code_(code), msg_(msg)
    {
    }

    windows_error(const windows_error& e) : code_(e.code_), msg_(e.msg_)
    {
    }

    ~windows_error() throw() // virtual
    {
    }

    windows_error& operator=(const windows_error& e)
    {
        code_ = e.code_;
        msg_ = e.msg_;
        what_.clear();
    }

    const char* what() const throw() // virtual
    {
        if (what_.empty())
        {
            try
            {
                std::string tmp(msg_);
                if (!tmp.empty())
                    tmp += ": ";
                tmp += get_windows_error_message(code_);
                what_.swap(tmp);
            }
            catch (...)
            {
                return msg_;
            }
        }
        return what_.c_str();
    }

    unsigned long code() const
    {
        return code_;
    }

private:
    unsigned long code_;
    const char* msg_;
    mutable std::string what_;
};

} } // End namespaces system, hamigaki.

#endif // HAMIGAKI_SYSTEM_WINDOWS_ERROR_HPP
