// sign_bit.hpp: test for sign

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/math for library home page.

#ifndef HAMIGAKI_MATH_SIGN_BIT_HPP
#define HAMIGAKI_MATH_SIGN_BIT_HPP

#include <boost/config.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/cstdint.hpp>
#include <limits>
#include <math.h>

namespace hamigaki { namespace math {

namespace detail
{

#if defined(signbit)

template<class T>
inline bool sign_bit_helper(T t, const boost::true_type&)
{
    return signbit(t) != 0;
}

#elif defined(_M_IX86) || defined(_M_AMD64) || defined(__i386__)

template<class T>
inline bool sign_bit_helper_for_float(T t, const boost::mpl::size_t<4>&)
{
    return (*reinterpret_cast<boost::uint32_t*>(&t) & 0x80000000) != 0;
}

template<class T>
inline bool sign_bit_helper_for_float(T t, const boost::mpl::size_t<8>&)
{
    return (reinterpret_cast<boost::uint32_t*>(&t)[1] & 0x80000000) != 0;
}

template<class T>
inline bool sign_bit_helper_for_float(T t, const boost::mpl::size_t<10>&)
{
    return (reinterpret_cast<boost::uint16_t*>(&t)[4] & 0x8000) != 0;
}

template<class T>
inline bool sign_bit_helper_for_float(T t, const boost::mpl::size_t<12>&)
{
    return (reinterpret_cast<boost::uint32_t*>(&t)[2] & 0x00008000) != 0;
}

template<class T>
inline bool sign_bit_helper_for_float(T t, const boost::mpl::size_t<16>&)
{
    return (reinterpret_cast<boost::uint32_t*>(&t)[2] & 0x00008000) != 0;
}

template<class T>
inline bool sign_bit_helper(T t, const boost::true_type&)
{
    return sign_bit_helper_for_float(t, ::boost::mpl::size_t<sizeof(T)>());
}

#elif defined(__POWERPC__) || defined(__ppc__)

template<class T>
inline bool sign_bit_helper(T t, const boost::true_type&)
{
    return (*reinterpret_cast<boost::uint32_t*>(&t) & 0x80000000) != 0;
}

#else
    #error "Sorry, unsupported architecture"
#endif

template<class T>
inline bool sign_bit_helper_for_int(T t, const boost::true_type&)
{
    return t < T();
}

template<class T>
inline bool sign_bit_helper_for_int(T t, const boost::false_type&)
{
    return false;
}

template<class T>
inline bool sign_bit_helper(T t, const boost::false_type&)
{
    return detail::sign_bit_helper_for_int(
        t,
        boost::integral_constant<bool, std::numeric_limits<T>::is_signed>()
    );
}

} // namespace detail

template<class T>
inline bool sign_bit(T t)
{
    return detail::sign_bit_helper(t, ::boost::is_floating_point<T>());
}

} } // End namespaces math, hamigaki.

#endif // HAMIGAKI_MATH_SIGN_BIT_HPP
