// dec_format.hpp: decimal formatting

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DEC_FORMAT_HPP
#define HAMIGAKI_DEC_FORMAT_HPP

#include <hamigaki/static_widen.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <limits>
#include <stdexcept>
#include <string>

namespace hamigaki {

template<class CharT>
struct dec_traits;

template<>
struct dec_traits<char>
{
    static char to_dec(int n)
    {
        return "0123456789"[n];
    }

    static int from_dec(char c)
    {
        if ((c < '0') || (c > '9'))
            throw std::runtime_error("bad decimal character");
        return c - '0';
    }

    static const char* zero()
    {
        return "0";
    }
};

template<>
struct dec_traits<wchar_t>
{
    static wchar_t to_dec(int n)
    {
        return L"0123456789"[n];
    }

    static int from_dec(wchar_t c)
    {
        if ((c < L'0') || (c > L'9'))
            throw std::runtime_error("bad decimal character");
        return c - L'0';
    }

    static const wchar_t* zero()
    {
        return L"0";
    }
};

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
template<>
struct dec_traits<const char> : dec_traits<char> {};

template<>
struct dec_traits<const wchar_t> : dec_traits<wchar_t> {};
#endif

namespace detail {

template<typename CharT, typename T>
inline std::basic_string<CharT> to_dec_impl(T n, boost::mpl::bool_<false>)
{
    if (n == T())
        return dec_traits<CharT>::zero();

    std::basic_string<CharT> s;
    while (n)
    {
        s += dec_traits<CharT>::to_dec(static_cast<int>(n % 10));
        n /= 10;
    }
    std::reverse(s.begin(), s.end());
    return s;
}

template<typename CharT, typename T>
inline std::basic_string<CharT> to_dec_impl(T n, boost::mpl::bool_<true>)
{
    if (n == T())
        return dec_traits<CharT>::zero();

    std::basic_string<CharT> s;
    if (n < T())
    {
        while (n)
        {
            T tmp = n % 10;
            if (tmp > T())
                tmp -= 10;
            s += dec_traits<CharT>::to_dec(static_cast<int>(-tmp));
            n /= 10;
        }
        s.push_back(static_widen<CharT,'-'>::value);
    }
    else
    {
        while (n)
        {
            s += dec_traits<CharT>::to_dec(static_cast<int>(n % 10));
            n /= 10;
        }
    }
    std::reverse(s.begin(), s.end());
    return s;
}

template<typename T, typename CharT>
inline T from_dec_impl(
    const CharT* first, const CharT* last, boost::mpl::bool_<false>)
{
    T tmp = 0;
    while (first != last)
    {
        tmp *= 10;
        tmp += dec_traits<CharT>::from_dec(*(first++));
    }
    return tmp;
}

template<typename T, typename CharT>
inline T from_dec_impl(
    const CharT* first, const CharT* last, boost::mpl::bool_<true>)
{
    if (first == last)
        return T();

    if (*first == static_widen<CharT,'-'>::value)
    {
        ++first;
        T tmp = 0;
        while (first != last)
        {
            tmp *= 10;
            tmp -= dec_traits<CharT>::from_dec(*(first++));
        }
        return tmp;
    }
    else
    {
        T tmp = 0;
        while (first != last)
        {
            tmp *= 10;
            tmp += dec_traits<CharT>::from_dec(*(first++));
        }
        return tmp;
    }
}

} // namespace detail

template<typename CharT, typename T>
inline std::basic_string<CharT> to_dec(T n)
{
    return detail::to_dec_impl<CharT,T>(n,
        boost::mpl::bool_<std::numeric_limits<T>::is_signed>());
}

template<typename T, typename CharT>
inline T from_dec(const CharT* first, const CharT* last)
{
    return detail::from_dec_impl<T,CharT>(first, last,
        boost::mpl::bool_<std::numeric_limits<T>::is_signed>());
}

template<typename T, typename CharT>
inline T from_dec(const std::basic_string<CharT>& s)
{
    const CharT* p = s.c_str();
    return hamigaki::from_dec<T,CharT>(p, p + s.size());
}

} // End namespace hamigaki.

#endif // HAMIGAKI_DEC_FORMAT_HPP
