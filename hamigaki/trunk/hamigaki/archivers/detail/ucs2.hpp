//  ucs2.hpp: UCS-2 converter

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_UCS2_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_UCS2_HPP

#include <boost/config.hpp>
#include <hamigaki/binary/endian.hpp>
#include <boost/scoped_array.hpp>
#include <cstdlib>
#include <stdexcept>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    #define HAMIGAKI_ARCHIVERS_WINDOWS

    extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(
        unsigned, unsigned long, const wchar_t*, int,
        char*, int, const char*, int*);
#endif

namespace hamigaki { namespace archivers { namespace detail {

inline std::size_t wide_to_narrow(char* s, const wchar_t* pwcs, std::size_t n)
{
#if defined(HAMIGAKI_ARCHIVERS_WINDOWS)
    int res = ::WideCharToMultiByte(
        0, 0, pwcs, -1, s, static_cast<int>(n), 0, 0);
    if (res == 0)
        throw std::runtime_error("failed WideCharToMultiByte()");
    return static_cast<std::size_t>(res - 1);
#else
    std::size_t res = std::wcstombs(s, pwcs, n);
    if (res == static_cast<std::size_t>(-1))
        throw std::runtime_error("failed wcstombs()");
    return res;
#endif
}

inline std::string wide_to_narrow(const wchar_t* pwcs)
{
    std::size_t size = detail::wide_to_narrow(0, pwcs, 0);
    boost::scoped_array<char> buf(new char[size+1]);
    detail::wide_to_narrow(buf.get(), pwcs, size+1);
    return std::string(buf.get(), size);
}

inline std::size_t ucs2be_to_wide(wchar_t* pwcs, const char* s, std::size_t n)
{
    std::size_t src_size = n/2;
    for (std::size_t i = 0; i < src_size; ++i)
        pwcs[i] = hamigaki::decode_int<big,2>(s + i*2);
    return src_size*2;
}

inline std::string ucs2be_to_narrow(const char* s, std::size_t n)
{
    std::size_t src_size = n/2;
    boost::scoped_array<wchar_t> src(new wchar_t[src_size + 1]);
    detail::ucs2be_to_wide(src.get(), s, n);
    src[src_size] = 0;

    return detail::wide_to_narrow(src.get());
}

inline std::string ucs2be_to_narrow(const std::string& data)
{
    return detail::ucs2be_to_narrow(data.c_str(), data.size());
}

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_UCS2_HPP
