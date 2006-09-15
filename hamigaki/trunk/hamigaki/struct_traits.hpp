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
#include <boost/mpl/find.hpp>
#include <boost/mpl/iterator_range.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/size_t.hpp>

namespace hamigaki {

template<class T>
struct struct_traits
{
    typedef void members;
};

template<
    class Struct, class Type, Type Struct::* PtrToMember,
    endianness E=native
>
struct member
{
    typedef Struct class_type;
    typedef Type member_type;

    static const endianness endian = E;

    Type& operator()(Struct& x) const
    {
        return x.*PtrToMember;
    }

    const Type& operator()(const Struct& x) const
    { 
        return x.*PtrToMember;
    }
};

template<std::size_t Size>
struct padding
{
    typedef void class_type;
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
struct member_offset;

template<class Struct, class Type, Type Struct::* PtrToMember, endianness E>
struct member_offset<member<Struct, Type, PtrToMember, E> >
{
    typedef typename boost::mpl::accumulate<
        boost::mpl::iterator_range<
            typename boost::mpl::begin<
                typename struct_traits<Struct>::members
            >::type,
            typename boost::mpl::find<
                typename struct_traits<Struct>::members,
                member<Struct, Type, PtrToMember, E>
            >::type
        >,
        boost::mpl::size_t<0>,
        boost::mpl::plus<
            boost::mpl::_1,
            member_size<boost::mpl::_2>
        >
    >::type type;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_STRUCT_TRAITS_HPP
