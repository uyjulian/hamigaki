//  member_access_traits.hpp: type traits for member access

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/type_traits for library home page.

#ifndef HAMIGAKI_TYPE_TRAITS_MEMBER_ACCESS_TRAITS_HPP
#define HAMIGAKI_TYPE_TRAITS_MEMBER_ACCESS_TRAITS_HPP

#include <boost/mpl/if.hpp>
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

    typedef typename boost::mpl::if_<
        typename boost::is_const<class_type>::type,
        typename boost::mpl::if_<
            typename boost::is_volatile<class_type>::type,
            typename boost::add_cv<U>::type,
            typename boost::add_const<U>::type
        >::type,
        typename boost::mpl::if_<
            typename boost::is_volatile<class_type>::type,
            typename boost::add_volatile<U>::type,
            U
        >::type
    >::type member_type;

    typedef U value_type;
    typedef typename boost::add_reference<member_type>::type reference;
    typedef typename boost::add_pointer<member_type>::type pointer;
};

} // End namespaces hamigaki.

#endif // HAMIGAKI_TYPE_TRAITS_MEMBER_ACCESS_TRAITS_HPP
