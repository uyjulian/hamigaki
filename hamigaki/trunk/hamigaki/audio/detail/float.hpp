//  float.hpp: float codec

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_FLOAT_HPP
#define HAMIGAKI_AUDIO_DETAIL_FLOAT_HPP

#include <boost/cstdint.hpp>
#include <cmath>
#include <cstdio>
#include <limits>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <math.h>
#endif

namespace hamigaki { namespace audio { namespace detail {

template<typename LeastFloat>
struct float_fast_t { typedef LeastFloat fast; };

template<int Category> struct float_least_helper {};

template<> struct float_least_helper<1> { typedef long double least; };
template<> struct float_least_helper<2> { typedef double least; };
template<> struct float_least_helper<3> { typedef float least; };

template<int Bits>
struct float_t
{
    typedef typename float_least_helper
        <
            (Bits <= std::numeric_limits<long double>::digits) +
            (Bits <= std::numeric_limits<double>::digits) +
            (Bits <= std::numeric_limits<float>::digits)
        >::least least;
    typedef typename float_fast_t<least>::fast fast;
};

enum float_format
{
    ieee754_single,
    ieee754_double
};

template <float_format Format>
struct float_traits;

template <>
struct float_traits<ieee754_single>
{
    typedef boost::uint_least32_t uint_type;
    typedef boost::int_least32_t mant_type;

    static const int bits = 32;
    static const int digits = 24;
    static const int exponent_digits = 8;
    static const int exponent_bias = 127;
};

template <>
struct float_traits<ieee754_double>
{
    typedef boost::uint_least64_t uint_type;
    typedef boost::int_least64_t mant_type;

    static const int bits = 64;
    static const int digits = 53;
    static const int exponent_digits = 11;
    static const int exponent_bias = 1023;
};

template<typename T>
inline bool sign_bit(T x)
{
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    return signbit(x)
#else
    if (x < T())
        return true;
    else if (x > T())
        return false;
    else
    {
        char buf[16];
        std::sprintf(buf, "%g", static_cast<double>(x));
        return buf[0] == '-';
    }
#endif
}

template<typename T>
inline bool is_nan(T x)
{
    return x != x;
}


template<typename T, float_format Format>
inline T decode_ieee754_impl(typename float_traits<Format>::uint_type n)
{
    typedef float_traits<Format> traits;
    typedef typename traits::uint_type uint_type;
    const int exp_dig = traits::exponent_digits;
    const int dig = traits::digits;

    const int exp_mask = (1 << exp_dig) - 1;
    int exp = static_cast<int>(n >> (dig-1)) & exp_mask;

    typedef typename traits::mant_type mant_t;
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    uint_type mant_mask = 1;
    mant_mask <<= (dig-1);
    --mant_mask;
#else
    const uint_type mant_mask = (static_cast<uint_type>(1) << (dig-1)) - 1;
#endif
    mant_t mant = static_cast<mant_t>(n & mant_mask);

    if ((exp == 0) && (mant == 0))
        return T();
    else if (exp == exp_mask)
    {
        if (mant == 0)
            return std::numeric_limits<T>::infinity();
        else
            return std::numeric_limits<T>::quiet_NaN();
    }

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    mant_t mant1 = 1;
    mant |= mant1 << (dig-1);
#else
    mant |= static_cast<mant_t>(1) << (dig-1);
#endif

    return std::ldexp(static_cast<T>(mant), exp-(traits::exponent_bias+dig-1));
}

template<typename T, float_format Format>
inline T decode_ieee754(typename float_traits<Format>::uint_type n)
{
    typedef float_traits<Format> traits;
    typedef typename traits::uint_type uint_type;
    const int bits = traits::bits;
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    uint_type sign_mask = 1;
    sign_mask <<= (bits-1);
#else
    const uint_type sign_mask = static_cast<uint_type>(1) << (bits-1);
#endif

    T tmp = decode_ieee754_impl<T,Format>(n);
    bool sign = (n & sign_mask) != 0;
    return sign ? -tmp : tmp;
}

template<typename T, float_format Format>
inline typename float_traits<Format>::uint_type encode_ieee754_impl(T x)
{
    typedef float_traits<Format> traits;
    typedef typename traits::uint_type uint_type;
    const int exp_dig = traits::exponent_digits;
    const int dig = traits::digits;

    const uint_type exp_mask = (1 << exp_dig) - 1;

    if (x == T())
        return 0;
    else if (x == std::numeric_limits<T>::infinity())
        return exp_mask << (dig-1);
    else if (is_nan(x))
    {
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
        uint_type tmp = 1;
        tmp <<= (dig-2);
        return (exp_mask << (dig-1)) | tmp;
#else
        return
            (exp_mask << (dig-1)) |
            (static_cast<uint_type>(1) << (dig-2));
#endif
    }

    int exp;
    typedef typename traits::mant_type mant_t;
    mant_t mant =
        static_cast<mant_t>(std::ldexp(std::frexp(x, &exp), dig));
    exp += (traits::exponent_bias-1);
    if (mant < mant_t())
        mant = -mant;

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    uint_type mant_mask = 1;
    mant_mask <<= (dig-1);
    --mant_mask;

    const uint_type long_exp = exp;
    const uint_type long_mant = mant;

    return (long_exp << (dig-1)) | (long_mant & mant_mask) ;
#else
    const uint_type mant_mask = (static_cast<uint_type>(1) << (dig-1)) - 1;
    return
        (static_cast<uint_type>(exp) << (dig-1)) |
        (static_cast<uint_type>(mant) & mant_mask) ;
#endif
}

template<typename T, float_format Format>
inline typename float_traits<Format>::uint_type encode_ieee754(T x)
{
    typedef float_traits<Format> traits;
    typedef typename traits::uint_type uint_type;
    const int bits = traits::bits;
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    uint_type sign_mask = 1;
    sign_mask <<= (bits-1);
#else
    const uint_type sign_mask = static_cast<uint_type>(1) << (bits-1);
#endif

    uint_type tmp = encode_ieee754_impl<T,Format>(std::abs(x));
    if (sign_bit(x))
        tmp |= sign_mask;
    return tmp;
}

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_FLOAT_HPP
