// iconv.hpp: iconv wrapper

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/charset for library home page.

#ifndef HAMIGAKI_CHARSET_DETAIL_ICONV_HPP
#define HAMIGAKI_CHARSET_DETAIL_ICONV_HPP

#include <stdexcept>
#include <iconv.h>

namespace hamigaki { namespace charset { namespace detail {

inline std::size_t indirect_iconv(
    size_t (*func)(iconv_t, char**, size_t*, char**, size_t*),
    iconv_t cd,
    char** inbuf, std::size_t* inbytesleft,
    char** outbuf, std::size_t* outbytesleft)
{
    return (*func)(cd, inbuf, inbytesleft, outbuf, outbytesleft);
}

inline std::size_t indirect_iconv(
    size_t (*func)(iconv_t, const char**, size_t*, char**, size_t*),
    iconv_t cd,
    char** inbuf, std::size_t* inbytesleft,
    char** outbuf, std::size_t* outbytesleft)
{
    return (*func)(cd,
        const_cast<const char**>(inbuf), inbytesleft, outbuf, outbytesleft);
}

class iconv_wrapper
{
public:
    static const std::size_t error = static_cast<std::size_t>(-1);

    iconv_wrapper(const char* tocode, const char* fromcode)
        : handle_(::iconv_open(tocode, fromcode))
    {
        if (handle_ == reinterpret_cast<iconv_t>(-1))
            throw std::runtime_error("failed iconv_open()");
    }

    ~iconv_wrapper()
    {
        ::iconv_close(handle_);
    }

    std::size_t convert(
        char*& inbuf, std::size_t& inbytesleft,
        char*& outbuf, std::size_t& outbytesleft)
    {
        return indirect_iconv(
            &::iconv, handle_, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    }

    void flush(char*& outbuf, std::size_t& outbytesleft)
    {
        if (::iconv(handle_, 0, 0, &outbuf, &outbytesleft) == error)
            throw std::runtime_error("failed iconv()");
    }

    void reset()
    {
        ::iconv(handle_, 0, 0, 0, 0);
    }

private:
    iconv_t handle_;

    iconv_wrapper(const iconv_wrapper&);
    iconv_wrapper& operator=(const iconv_wrapper&);
};

} } } // End namespaces detail, charset, hamigaki.

#endif // HAMIGAKI_CHARSET_DETAIL_ICONV_HPP
