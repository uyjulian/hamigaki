// mu_law.hpp: mu-law codec

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_MU_LAW_HPP
#define HAMIGAKI_AUDIO_DETAIL_MU_LAW_HPP

#include <boost/cstdint.hpp>
#include <boost/version.hpp>
#include <cmath>

#if BOOST_VERSION >= 103500
    #include <boost/math/special_functions/log1p.hpp>
    #include <boost/math/special_functions/powm1.hpp>
#endif

namespace hamigaki { namespace audio { namespace detail {

template<typename T>
inline T decode_mu_law(boost::uint8_t n)
{
    static const T mu = static_cast<T>(255);

    bool minus = (n & 0x80) == 0;
    T x = static_cast<T>(~n & 0x7F)/127;
#if BOOST_VERSION < 103500
    T y = static_cast<T>(1)/mu*(std::pow(1+mu, x)-1);
#else
    T y = static_cast<T>(1)/mu*(boost::math::powm1(1+mu, x));
#endif

    if (minus)
        return -y;
    else
        return y;
}

template<typename T>
inline boost::uint8_t encode_mu_law(T x)
{
    static const T mu = static_cast<T>(255);

    bool minus = (x < T());

#if BOOST_VERSION < 103500
    T y = std::log(1+mu*std::abs(x))/std::log(1+mu);
#else
    T y = boost::math::log1p(mu*std::abs(x))/std::log(1+mu);
#endif
    if (y > 1)
        y = static_cast<T>(1);

    if (minus)
        return ~static_cast<unsigned>(127 * y) & 0x7F;
    else
        return ~static_cast<unsigned>(127 * y);
}

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_MU_LAW_HPP
