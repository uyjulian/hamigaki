//  struct_traits.hpp: type trsits for struct

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/

#ifndef HAMIGAKI_STRUCT_TRAITS_HPP
#define HAMIGAKI_STRUCT_TRAITS_HPP

#include <hamigaki/endian.hpp>
#include <boost/mpl/accumulate.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/iterator_range.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/type_traits/is_convertible.hpp>

namespace hamigaki {

template<class T>
struct struct_traits
{
    typedef void members;
};

template<class Struct, class Type, Type Struct::* PtrToMember>
struct member_base
{
    typedef Struct struct_type;
    typedef Type member_type;

    Type& operator()(Struct& x) const
    {
        return x.*PtrToMember;
    }

    const Type& operator()(const Struct& x) const
    { 
        return x.*PtrToMember;
    }
};

template<
    class Struct, class Type, Type Struct::* PtrToMember,
    endianness E=native
>
struct member : member_base<Struct,Type,PtrToMember>
{
    static const endianness endian = E;
};

template<std::size_t Size>
struct padding
{
    typedef void struct_type;
    typedef void member_type;

    static const endianness endian = native;

    template<class Struct>
    void operator()(const Struct& x) const
    {
    }
};

template<class T>
struct struct_size;

namespace detail
{

template<class T, class Members>
struct sizeof_impl
{
    typedef typename struct_size<T>::type type;
};

template<class T>
struct sizeof_impl<T,void>
{
    typedef boost::mpl::size_t<sizeof(T)> type;
};

} // namespace detail

template<class T>
struct member_size;

template<class Struct, class Type, Type Struct::* PtrToMember, endianness E>
struct member_size<member<Struct, Type, PtrToMember, E> >
{
    typedef typename detail::sizeof_impl<
        Type,
        typename struct_traits<Type>::members
    >::type type;
};

template<std::size_t Size>
struct member_size<padding<Size> >
{
    typedef boost::mpl::size_t<Size> type;
};


template<class T>
struct struct_size
{
    typedef typename boost::mpl::accumulate<
        typename struct_traits<T>::members,
        boost::mpl::size_t<0>,
        boost::mpl::plus<
            boost::mpl::_1,
            member_size<boost::mpl::_2>
        >
    >::type type;
};


template<class T>
struct binary_size
{
    typedef typename detail::sizeof_impl<
        T,
        typename struct_traits<T>::members
    >::type type;
};


template<class T>
struct member_offset
{
    typedef typename boost::mpl::accumulate<
        boost::mpl::iterator_range<
            typename boost::mpl::begin<
                typename struct_traits<typename T::struct_type>::members
            >::type,
            typename boost::mpl::find_if<
                typename struct_traits<typename T::struct_type>::members,
                boost::is_convertible<boost::mpl::_1,T>
            >::type
        >,
        boost::mpl::size_t<0>,
        boost::mpl::plus<
            boost::mpl::_1,
            member_size<boost::mpl::_2>
        >
    >::type type;
};

template<class Struct, class Type, Type Struct::* PtrToMember>
struct binary_offset
{
    typedef typename member_offset<
        member_base<Struct, Type, PtrToMember>
    >::type type;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_STRUCT_TRAITS_HPP
