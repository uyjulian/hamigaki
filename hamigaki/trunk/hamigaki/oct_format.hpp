// oct_format.hpp: octal formatting

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_OCT_FORMAT_HPP
#define HAMIGAKI_OCT_FORMAT_HPP

#include <hamigaki/static_widen.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <stdexcept>
#include <string>

namespace hamigaki {

template<class CharT>
struct oct_traits;

template<>
struct oct_traits<char>
{
    static char to_oct(int n)
    {
        return "01234567"[n];
    }

    static int from_oct(char c)
    {
        if ((c < '0') || (c > '7'))
            throw std::runtime_error("bad octal character");
        return c - '0';
    }
};

template<>
struct oct_traits<wchar_t>
{
    static wchar_t to_oct(int n)
    {
        return L"01234567"[n];
    }

    static int from_oct(wchar_t c)
    {
        if ((c < L'0') || (c > L'7'))
            throw std::runtime_error("bad octal character");
        return c - L'0';
    }
};

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
template<>
struct oct_traits<const char> : oct_traits<char> {};

template<>
struct oct_traits<const wchar_t> : oct_traits<wchar_t> {};
#endif

template<typename CharT, std::size_t Size, typename T>
inline std::basic_string<CharT> to_oct(T n)
{
    std::basic_string<CharT> s;
    for (std::size_t i = 0; i < Size; ++i)
    {
        s += oct_traits<CharT>::to_oct(static_cast<int>(n & 07));
        n >>= 3;
    }
    std::reverse(s.begin(), s.end());
    return s;
}

template<typename CharT, typename T>
inline std::basic_string<CharT> to_oct(T n)
{
    std::basic_string<CharT> s;
    while (n)
    {
        s += oct_traits<CharT>::to_oct(static_cast<int>(n & 07));
        n >>= 3;
    }
    s.push_back(static_widen<CharT,'0'>::value);
    std::reverse(s.begin(), s.end());
    return s;
}

template<typename T, typename CharT>
inline T from_oct(const CharT* first, const CharT* last)
{
    T tmp = 0;
    while (first != last)
    {
        tmp <<= 3;
        tmp |= oct_traits<CharT>::from_oct(*(first++));
    }
    return tmp;
}

} // End namespace hamigaki.

#endif // HAMIGAKI_OCT_FORMAT_HPP
