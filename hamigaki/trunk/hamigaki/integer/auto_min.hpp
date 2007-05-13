// auto_min.hpp: select the minimum value with automatic result type detection

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/integer

#ifndef HAMIGAKI_INTEGER_AUTO_MIN_HPP
#define HAMIGAKI_INTEGER_AUTO_MIN_HPP

#include <hamigaki/integer/detail/to_unsigned.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <limits>

namespace hamigaki {

namespace detail
{

template<typename T, typename U>
struct auto_min_result
{
    typedef typename boost::mpl::eval_if_c<
        std::numeric_limits<T>::is_signed,
        boost::mpl::eval_if_c<
            std::numeric_limits<U>::is_signed,
            boost::mpl::if_c<(sizeof(T) < sizeof(U)), U, T>,
            boost::mpl::identity<T>
        >,
        boost::mpl::eval_if_c<
            std::numeric_limits<U>::is_signed,
            boost::mpl::identity<U>,
            boost::mpl::if_c<(sizeof(T) < sizeof(U)), T, U>
        >
    >::type type;
};


template<typename T, typename U, bool SignT, bool SignU>
struct auto_min_impl
{
    typedef typename auto_min_result<T,U>::type result_type;

    result_type operator()(T t, U u) const
    {
        typedef typename boost::mpl::if_c<
            (sizeof(T) < sizeof(U)),
            U,
            T
        >::type value_type;

        value_type t2 = t;
        value_type u2 = u;
        return static_cast<result_type>(t2 < u2 ? t2 : u2);
    }
};

template<typename T, typename U>
struct auto_min_impl<T,U,true,false>
{
    typedef T result_type;

    result_type operator()(T t, U u) const
    {
        if (t <= 0)
            return t;

        typedef typename to_unsigned<T>::type unsigned_type;
        typedef auto_min_impl<unsigned_type,U,false,false> impl_type;
        return static_cast<T>(impl_type()(static_cast<unsigned_type>(t), u));
    }
};

template<typename T, typename U>
struct auto_min_impl<T,U,false,true>
{
    typedef U result_type;

    result_type operator()(T t, U u) const
    {
        typedef auto_min_impl<U,T,true,false> impl_type;
        return impl_type()(u,t);
    }
};

} // namespace detail

template<typename T, typename U>
inline typename detail::auto_min_result<T,U>::type auto_min(T t, U u)
{
    BOOST_MPL_ASSERT(( boost::is_integral<T> ));
    BOOST_MPL_ASSERT(( boost::is_integral<U> ));

    typedef detail::auto_min_impl<
        T, U,
        std::numeric_limits<T>::is_signed,
        std::numeric_limits<U>::is_signed
    > impl_type;

    return impl_type()(t,u);
}

} // End namespace hamigaki.

#endif // HAMIGAKI_INTEGER_AUTO_MIN_HPP
