// float.hpp: float codec

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_FLOAT_HPP
#define HAMIGAKI_AUDIO_DETAIL_FLOAT_HPP

#include <boost/cstdint.hpp>

namespace hamigaki { namespace audio { namespace detail {

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
    static const int bits = 32;

    typedef float value_type;
    typedef boost::uint32_t uint_type;
};

template <>
struct float_traits<ieee754_double>
{
    static const int bits = 64;

    typedef double value_type;
    typedef boost::uint64_t uint_type;
};


template<typename T, float_format Format>
inline T decode_ieee754(typename float_traits<Format>::uint_type n)
{
    typedef float_traits<Format> traits;
    typedef typename traits::uint_type uint_type;
    typedef typename traits::value_type value_type;
    return static_cast<T>(*reinterpret_cast<value_type*>(&n));
}


template<typename T, float_format Format>
inline typename float_traits<Format>::uint_type encode_ieee754(T x)
{
    typedef float_traits<Format> traits;
    typedef typename traits::uint_type uint_type;
    typedef typename traits::value_type value_type;
    value_type tmp = static_cast<value_type>(x);
    return *reinterpret_cast<uint_type*>(&tmp);
}

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_FLOAT_HPP
