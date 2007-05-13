// is_nan.hpp: test for NaN

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/math for library home page.

// Original source:
// http://lists.boost.org/Archives/boost/2006/07/108456.php
// http://lists.boost.org/Archives/boost/2006/07/108462.php

#ifndef HAMIGAKI_MATH_IS_NAN_HPP
#define HAMIGAKI_MATH_IS_NAN_HPP

#include <boost/config.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <limits>
#include <math.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
    #include <float.h>
#endif

namespace hamigaki { namespace math {

namespace detail
{

#if defined(isnan)

template<class T>
inline bool is_nan_helper(T t, const boost::true_type&)
{
    return isnan(t) != 0;
}

#elif defined(_MSC_VER) || defined(__BORLANDC__)

template<class T>
inline bool is_nan_helper(T t, const boost::true_type&)
{
    return ::_isnan(static_cast<double>(t)) != 0;
}

#if defined(__BORLANDC__)
inline bool is_nan_helper(long double t, const boost::true_type&)
{
    return ::_isnanl(t) != 0;
}
#endif

#else

template<class T>
inline bool is_nan_helper_impl(T t, const boost::true_type&)
{
    return !(t <= std::numeric_limits<T>::infinity());
}

template<class T>
inline bool is_nan_helper_impl(T t, const boost::false_type&)
{
    return t != t;
}

template<class T>
inline bool is_nan_helper(T t, const boost::true_type&)
{
    return detail::is_nan_helper_impl(
        t,
        boost::integral_constant<bool, std::numeric_limits<T>::has_infinity>()
    );
}

#endif

template<class T>
inline bool is_nan_helper(T t, const boost::false_type&)
{
    return false;
}

} // namespace detail

template<class T>
inline bool is_nan(T t)
{
    return detail::is_nan_helper(t, ::boost::is_floating_point<T>());
}

} } // End namespaces math, hamigaki.

#endif // HAMIGAKI_MATH_IS_NAN_HPP
