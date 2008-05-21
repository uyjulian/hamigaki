// a_law.hpp: A-law codec

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_A_LAW_HPP
#define HAMIGAKI_AUDIO_DETAIL_A_LAW_HPP

#include <boost/cstdint.hpp>
#include <cmath>

namespace hamigaki { namespace audio { namespace detail {

template<typename T>
inline T decode_a_law(boost::uint8_t n)
{
    n ^= 0x55;

    bool minus = (n & 0x80) == 0;

    int high = (n >> 4) & 0x07;
    int low = n & 0x0F;

    T m;
    int exp;

    if (high == 0)
    {
        m = static_cast<T>((low<<1) + 1);
        exp = -12;
    }
    else
    {
        m = static_cast<T>((low<<1) + (32 + 1));
        exp = high - 13;
    }

    if (minus)
        m = -m;
    return std::ldexp(m, exp);
}

template<typename T>
inline boost::uint8_t encode_a_law_impl(T x)
{
    if (x == 0)
        return 0x80;

    int exp;
    T m = std::frexp(x, &exp);

    if (exp > 0)
    {
        if (m < 0)
            return 0x7F;
        else
            return 0xFF;
    }

    unsigned n;
    if (exp <= -7)
        n = static_cast<unsigned>(static_cast<int>(std::abs(m)*(1<<(exp+11))));
    else
    {
        n = (static_cast<unsigned>(exp+7) << 4) |
            static_cast<unsigned>(static_cast<int>(std::abs(m)*32)-16) ;
    }

    if (x >= 0)
        n |= 0x80;

    return static_cast<boost::uint8_t>(n);
}

template<typename T>
inline boost::uint8_t encode_a_law(T x)
{
    return static_cast<boost::uint8_t>(encode_a_law_impl<T>(x) ^ 0x55);
}

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_A_LAW_HPP
