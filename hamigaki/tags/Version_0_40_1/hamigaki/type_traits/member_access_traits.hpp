// member_access_traits.hpp: type traits for member access

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/type_traits for library home page.

#ifndef HAMIGAKI_TYPE_TRAITS_MEMBER_ACCESS_TRAITS_HPP
#define HAMIGAKI_TYPE_TRAITS_MEMBER_ACCESS_TRAITS_HPP

#include <boost/detail/workaround.hpp>

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    #include <hamigaki/type_traits/detail/borland/member_access_traits.hpp>
#else

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_cv.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/add_volatile.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_volatile.hpp>
#include <boost/type_traits/remove_reference.hpp>

namespace hamigaki {

template<class T, class U>
struct member_access_traits
{
    typedef typename boost::remove_reference<T>::type class_type;

    typedef typename boost::mpl::eval_if<
        boost::is_const<class_type>,
        boost::mpl::eval_if<
            boost::is_volatile<class_type>,
            boost::add_cv<U>,
            boost::add_const<U>
        >,
        boost::mpl::eval_if<
            boost::is_volatile<class_type>,
            boost::add_volatile<U>,
            boost::mpl::identity<U>
        >
    >::type member_type;

    typedef U value_type;
    typedef typename boost::add_reference<member_type>::type reference;
    typedef typename boost::add_pointer<member_type>::type pointer;
};

} // End namespace hamigaki.

#endif

#endif // HAMIGAKI_TYPE_TRAITS_MEMBER_ACCESS_TRAITS_HPP
