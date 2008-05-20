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
    bool minus = (n & 0x80) == 0;

    int exp = static_cast<int>((n >> 4) & 0x07) - 7;
    T m = static_cast<T>((n & 0x0F)+16)/32;
    if (minus)
        m = -m;
    return std::ldexp(m, exp);
}

template<typename T>
inline boost::uint8_t encode_a_law(T x)
{
    if (x == 0)
        return 0x80;

    int exp;
    T m = std::frexp(x, &exp);

    if (exp < -7)
    {
        if (m < 0)
            return 0x00;
        else
            return 0x80;
    }
    else if (exp > 0)
    {
        if (m < 0)
            return 0x7F;
        else
            return 0xFF;
    }

    unsigned n =
        (static_cast<unsigned>(exp+7) << 4) |
        static_cast<unsigned>(static_cast<int>(std::abs(m)*32)-16) ;

    if (m >= 0)
        n |= 0x80;

    return static_cast<boost::uint8_t>(n);
}

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_A_LAW_HPP
