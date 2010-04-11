// posix_error.hpp: POSIX error class

// Copyright Takeshi Mouri 2007-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/system for library home page.

#ifndef HAMIGAKI_SYSTEM_POSIX_ERROR_HPP
#define HAMIGAKI_SYSTEM_POSIX_ERROR_HPP

#include <hamigaki/system/system_error.hpp>
#include <cstring>
#include <new>
#include <errno.h>
#include <string.h>

#if defined(__CYGWIN__)
    #define HAMIGAKI_SYSTEM_USE_GNU_STRERROR_R
#elif defined(__GLIBC__)
    #if defined(_GNU_SOURCE)
        #define HAMIGAKI_SYSTEM_USE_GNU_STRERROR_R
    #elif defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600)
        #define HAMIGAKI_SYSTEM_USE_STRERROR_R
    #elif defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L)
        #define HAMIGAKI_SYSTEM_USE_STRERROR_R
    #else
        #define HAMIGAKI_SYSTEM_USE_GNU_STRERROR_R
    #endif
#elif defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600)
    #define HAMIGAKI_SYSTEM_USE_STRERROR_R
#elif defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L)
    #define HAMIGAKI_SYSTEM_USE_STRERROR_R
#endif

namespace hamigaki { namespace system {

struct posix_error_traits
{
    typedef int value_type;

    static std::string message(int code)
    {
        std::string tmp;
#if defined(HAMIGAKI_SYSTEM_USE_GNU_STRERROR_R)
        char buf[256];
        tmp = ::strerror_r(code, buf, sizeof(buf));
#elif defined(HAMIGAKI_SYSTEM_USE_STRERROR_R)
        char buf[64];
        int res = ::strerror_r(code, buf, sizeof(buf));
#if defined(__GLIBC__)
        if (res == -1)
            res = errno;
#endif
        if (res == 0)
            tmp = buf;
        else if (res == ERANGE)
        {
            std::size_t size = sizeof(buf) << 1;
            while (char* p = new(std::nothrow) char[size])
            {
                res = ::strerror_r(code, p, size);
#if defined(__GLIBC__)
                if (res == -1)
                    res = errno;
#endif
                if (res == 0)
                {
                    try
                    {
                        tmp = p;
                    }
                    catch (...)
                    {
                        delete[] p;
                        throw;
                    }
                }
                delete[] p;
                if (res == ERANGE)
                    size <<= 1;
                else
                    break;
            }
        }
#else
        tmp = std::strerror(code);
#endif
        return tmp;
    }
};

typedef system_error<posix_error_traits> posix_error;

} } // End namespaces system, hamigaki.

#endif // HAMIGAKI_SYSTEM_POSIX_ERROR_HPP
