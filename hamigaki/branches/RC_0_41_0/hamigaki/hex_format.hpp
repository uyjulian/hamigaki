// hex_format.hpp: hexadecimal formatting

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_HEX_FORMAT_HPP
#define HAMIGAKI_HEX_FORMAT_HPP

#include <boost/detail/workaround.hpp>
#include <boost/array.hpp>
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

template<typename CharT, class T>
struct to_hex_helper
{
    static std::basic_string<CharT> convert(T n, bool is_upper)
    {
        std::basic_string<CharT> s;
        for (std::size_t i = 0; i < sizeof(n); ++i)
        {
            boost::uint8_t tmp =
                static_cast<boost::uint8_t>((n >> ((sizeof(n)-1-i)*8)) & 0xFFu);
            s += hex_traits<CharT>::to_hex(tmp / 16, is_upper);
            s += hex_traits<CharT>::to_hex(tmp % 16, is_upper);
        }
        return s;
    }
};

template<typename CharT>
struct to_hex_helper<CharT, boost::uint8_t>
{
    static std::basic_string<CharT> convert(boost::uint8_t n, bool is_upper)
    {
        std::basic_string<CharT> s;
        s += hex_traits<CharT>::to_hex(n / 16, is_upper);
        s += hex_traits<CharT>::to_hex(n % 16, is_upper);
        return s;
    }
};

template<typename CharT, std::size_t N>
struct to_hex_helper<
    CharT, boost::array<boost::uint8_t,N>
>
{
    static std::basic_string<CharT> convert(
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
};

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

template<typename CharT, class T>
inline std::basic_string<CharT> to_hex(const T& x, bool is_upper)
{
    return to_hex_helper<CharT,T>::convert(x, is_upper);
}

template<typename CharT>
inline std::pair<CharT,CharT> to_hex_pair(boost::uint8_t n, bool is_upper)
{
    return std::pair<CharT,CharT>(
        hex_traits<CharT>::to_hex(n / 16, is_upper),
        hex_traits<CharT>::to_hex(n % 16, is_upper)
    );
};

template<typename CharT>
inline boost::uint8_t from_hex(CharT c1, CharT c2)
{
    return static_cast<boost::uint8_t>(
        hex_traits<CharT>::from_hex(c1) * 16 +
        hex_traits<CharT>::from_hex(c2)
    );
}

template<typename CharT>
inline boost::uint8_t from_hex(const CharT (&s)[2])
{
    return static_cast<boost::uint8_t>(
        hex_traits<CharT>::from_hex(s[0]) * 16 +
        hex_traits<CharT>::from_hex(s[1])
    );
}

template<typename CharT>
inline boost::uint16_t from_hex(const CharT (&s)[4])
{
    using boost::uint16_t;

    return
        (static_cast<uint16_t>(hex_traits<CharT>::from_hex(s[0])) << 12) |
        (static_cast<uint16_t>(hex_traits<CharT>::from_hex(s[1])) <<  8) |
        (static_cast<uint16_t>(hex_traits<CharT>::from_hex(s[2])) <<  4) |
        (static_cast<uint16_t>(hex_traits<CharT>::from_hex(s[3]))      ) ;
}

template<typename CharT>
inline boost::uint32_t from_hex(const CharT (&s)[8])
{
    using boost::uint32_t;

    return
        (static_cast<uint32_t>(hex_traits<CharT>::from_hex(s[0])) << 28) |
        (static_cast<uint32_t>(hex_traits<CharT>::from_hex(s[1])) << 24) |
        (static_cast<uint32_t>(hex_traits<CharT>::from_hex(s[2])) << 20) |
        (static_cast<uint32_t>(hex_traits<CharT>::from_hex(s[3])) << 16) |
        (static_cast<uint32_t>(hex_traits<CharT>::from_hex(s[4])) << 12) |
        (static_cast<uint32_t>(hex_traits<CharT>::from_hex(s[5])) <<  8) |
        (static_cast<uint32_t>(hex_traits<CharT>::from_hex(s[6])) <<  4) |
        (static_cast<uint32_t>(hex_traits<CharT>::from_hex(s[7]))      ) ;
}

} // End namespace hamigaki.

#endif // HAMIGAKI_HEX_FORMAT_HPP
