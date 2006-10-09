//  hex_format.hpp: hexadecimal formatting

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_HEX_FORMAT_HPP
#define HAMIGAKI_HEX_FORMAT_HPP

#include <boost/detail/workaround.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <stdexcept>
#include <string>

namespace hamigaki {

template<class CharT>
struct hex_traits;

template<>
struct hex_traits<char>
{
    static char to_hex(int n, bool is_upper)
    {
        if (is_upper)
            return "0123456789ABCDEF"[n];
        else
            return "0123456789abcdef"[n];
    }

    static int from_hex(char c)
    {
        if ((c >= '0') && (c <= '9'))
            return c - '0';
        else
        {
            switch (c)
            {
                default:
                    throw std::runtime_error("bad hexadecimal character");
                case 'A': case 'a':
                    return 10;
                case 'B': case 'b':
                    return 11;
                case 'C': case 'c':
                    return 12;
                case 'D': case 'd':
                    return 13;
                case 'E': case 'e':
                    return 14;
                case 'F': case 'f':
                    return 15;
            }
        }
    }
};

template<>
struct hex_traits<wchar_t>
{
    static wchar_t to_hex(int n, bool is_upper)
    {
        if (is_upper)
            return L"0123456789ABCDEF"[n];
        else
            return L"0123456789abcdef"[n];
    }

    static int from_hex(wchar_t c)
    {
        if ((c >= L'0') && (c <= L'9'))
            return c - L'0';
        else
        {
            switch (c)
            {
                default:
                    throw std::runtime_error("bad hexadecimal character");
                case L'A': case L'a':
                    return 10;
                case L'B': case L'b':
                    return 11;
                case L'C': case L'c':
                    return 12;
                case L'D': case L'd':
                    return 13;
                case L'E': case L'e':
                    return 14;
                case L'F': case L'f':
                    return 15;
            }
        }
    }
};

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
template<>
struct hex_traits<const char> : hex_traits<char> {};

template<>
struct hex_traits<const wchar_t> : hex_traits<wchar_t> {};
#endif

template<typename CharT>
inline std::basic_string<CharT> to_hex(boost::uint8_t n, bool is_upper)
{
    std::basic_string<CharT> s;
    s += hex_traits<CharT>::to_hex(n / 16, is_upper);
    s += hex_traits<CharT>::to_hex(n % 16, is_upper);
    return s;
}

template<typename CharT>
inline std::basic_string<CharT> to_hex(
    boost::uint8_t n1, boost::uint8_t n2, bool is_upper)
{
    std::basic_string<CharT> s;
    s += hex_traits<CharT>::to_hex(n1 / 16, is_upper);
    s += hex_traits<CharT>::to_hex(n1 % 16, is_upper);
    s += hex_traits<CharT>::to_hex(n2 / 16, is_upper);
    s += hex_traits<CharT>::to_hex(n2 % 16, is_upper);
    return s;
}

template<typename CharT>
inline std::basic_string<CharT> to_hex(
    boost::uint8_t n1, boost::uint8_t n2,
    boost::uint8_t n3, boost::uint8_t n4, bool is_upper)
{
    std::basic_string<CharT> s;
    s += hex_traits<CharT>::to_hex(n1 / 16, is_upper);
    s += hex_traits<CharT>::to_hex(n1 % 16, is_upper);
    s += hex_traits<CharT>::to_hex(n2 / 16, is_upper);
    s += hex_traits<CharT>::to_hex(n2 % 16, is_upper);
    s += hex_traits<CharT>::to_hex(n3 / 16, is_upper);
    s += hex_traits<CharT>::to_hex(n3 % 16, is_upper);
    s += hex_traits<CharT>::to_hex(n4 / 16, is_upper);
    s += hex_traits<CharT>::to_hex(n4 % 16, is_upper);
    return s;
}

template<typename CharT, std::size_t N>
inline std::basic_string<CharT> to_hex(
    const boost::array<boost::uint8_t,N>& a, bool is_upper)
{
    std::basic_string<CharT> s;
    for (std::size_t i = 0; i < N; ++i)
    {
        boost::uint8_t n = a[i];
        s += hex_traits<CharT>::to_hex(n / 16, is_upper);
        s += hex_traits<CharT>::to_hex(n % 16, is_upper);
    }
    return s;
}

template<typename CharT>
inline boost::uint8_t from_hex(CharT c1, CharT c2)
{
    return static_cast<boost::uint8_t>(
        hex_traits<CharT>::from_hex(c1) * 16 +
        hex_traits<CharT>::from_hex(c2)
    );
}

} // End namespace hamigaki.

#endif // HAMIGAKI_HEX_FORMAT_HPP
