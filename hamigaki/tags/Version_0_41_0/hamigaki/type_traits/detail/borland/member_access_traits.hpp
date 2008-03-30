// member_access_traits.hpp: member_access_traits for Borland

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/type_traits for library home page.

#ifndef HAMIGAKI_TYPE_TRAITS_DETAIL_BORLAND_MEMBER_ACCESS_TRAITS_HPP
#define HAMIGAKI_TYPE_TRAITS_DETAIL_BORLAND_MEMBER_ACCESS_TRAITS_HPP

namespace hamigaki {

namespace detail
{

template<class T, class U>
struct member_access_traits_impl
{
    typedef U value_type;
    typedef typename U& reference;
    typedef typename U* pointer;
};

template<class T, class U>
struct member_access_traits_impl<const T, U>
{
    typedef U value_type;
    typedef const U& reference;
    typedef const U* pointer;
};

template<class T, class U>
struct member_access_traits_impl<volatile T, U>
{
    typedef U value_type;
    typedef volatile U& reference;
    typedef volatile U* pointer;
};

template<class T, class U>
struct member_access_traits_impl<const volatile T, U>
{
    typedef U value_type;
    typedef const volatile U& reference;
    typedef const volatile U* pointer;
};

template<class T, class U>
struct member_access_traits_impl<T, U&>
{
    typedef U& value_type;
    typedef U& reference;
    typedef U* pointer;
};

template<class T, class U>
struct member_access_traits_impl<const T, U&>
{
    typedef U& value_type;
    typedef U& reference;
    typedef U* pointer;
};

template<class T, class U>
struct member_access_traits_impl<volatile T, U&>
{
    typedef U& value_type;
    typedef U& reference;
    typedef U* pointer;
};

template<class T, class U>
struct member_access_traits_impl<const volatile T, U&>
{
    typedef U& value_type;
    typedef U& reference;
    typedef U* pointer;
};

} // namespace detail

template<class T, class U>
struct member_access_traits
    : detail::member_access_traits_impl<T,U>
{
};

template<class T, class U>
struct member_access_traits<T&,U>
    : detail::member_access_traits_impl<T,U>
{
};

template<class T, class U>
struct member_access_traits<const T&,U>
    : detail::member_access_traits_impl<const T,U>
{
};

template<class T, class U>
struct member_access_traits<volatile T&,U>
    : detail::member_access_traits_impl<volatile T,U>
{
};

template<class T, class U>
struct member_access_traits<const volatile T&,U>
    : detail::member_access_traits_impl<const volatile T,U>
{
};

} // End namespace hamigaki.

#endif // HAMIGAKI_TYPE_TRAITS_DETAIL_BORLAND_MEMBER_ACCESS_TRAITS_HPP
