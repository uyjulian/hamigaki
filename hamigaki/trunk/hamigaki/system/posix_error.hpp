// posix_error.hpp: POSIX error class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/system for library home page.

#ifndef HAMIGAKI_SYSTEM_POSIX_ERROR_HPP
#define HAMIGAKI_SYSTEM_POSIX_ERROR_HPP

#include <exception>
#include <new>
#include <string>
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

inline std::string get_posix_error_message(int code)
{
    std::string tmp;
#if defined(HAMIGAKI_SYSTEM_USE_GNU_STRERROR_R)
    char buf[256];
    tmp = ::strerror_r(code, buf, sizeof(buf));
#elif defined(HAMIGAKI_SYSTEM_USE_STRERROR_R)
    char buf[1];
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

class posix_error : public std::exception
{
public:
    posix_error() : code_(0)
    {
    }

    posix_error(unsigned long code, const char* msg)
        : code_(code), msg_(msg)
    {
    }

    posix_error(const posix_error& e) : code_(e.code_), msg_(e.msg_)
    {
    }

    ~posix_error() throw() // virtual
    {
    }

    posix_error& operator=(const posix_error& e)
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
                tmp += get_posix_error_message(code_);
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

#endif // HAMIGAKI_SYSTEM_POSIX_ERROR_HPP
