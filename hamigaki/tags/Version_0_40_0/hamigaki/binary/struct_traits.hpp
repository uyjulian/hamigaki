// struct_traits.hpp: type trsits for struct

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/binary

#ifndef HAMIGAKI_BINARY_STRUCT_TRAITS_HPP
#define HAMIGAKI_BINARY_STRUCT_TRAITS_HPP

#include <hamigaki/binary/endian.hpp>
#include <boost/mpl/accumulate.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/iterator_range.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/type_traits/is_convertible.hpp>

namespace hamigaki {

template<class T>
struct struct_traits;

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

template<class T>
struct sizeof_impl : struct_size<T>::type {};

template<class T, std::size_t Size>
struct sizeof_impl<T[Size]> : boost::mpl::size_t<sizeof_impl<T>::value*Size> {};

template<> struct sizeof_impl<char> : boost::mpl::size_t<1> {};
template<> struct sizeof_impl<signed char> : boost::mpl::size_t<1> {};
template<> struct sizeof_impl<unsigned char> : boost::mpl::size_t<1> {};

template<>
struct sizeof_impl<short> : boost::mpl::size_t<sizeof(short)>
{
};

template<>
struct sizeof_impl<unsigned short> : boost::mpl::size_t<sizeof(unsigned short)>
{
};

template<>
struct sizeof_impl<int> : boost::mpl::size_t<sizeof(int)>
{
};

template<>
struct sizeof_impl<unsigned int> : boost::mpl::size_t<sizeof(unsigned int)>
{
};

template<>
struct sizeof_impl<long> : boost::mpl::size_t<sizeof(long)>
{
};

template<>
struct sizeof_impl<unsigned long> : boost::mpl::size_t<sizeof(unsigned long)>
{
};

#if defined(BOOST_HAS_LONG_LONG)
template<>
struct sizeof_impl<long long>
    : boost::mpl::size_t<sizeof(long long)>
{
};

template<>
struct sizeof_impl<unsigned long long>
    : boost::mpl::size_t<sizeof(unsigned long long)>
{
};
#endif // defined(BOOST_HAS_LONG_LONG)

} // namespace detail

template<class T>
struct member_size;

template<class Struct, class Type, Type Struct::* PtrToMember, endianness E>
struct member_size<member<Struct, Type, PtrToMember, E> >
    : detail::sizeof_impl<Type>
{
};

template<std::size_t Size>
struct member_size<padding<Size> > : boost::mpl::size_t<Size> {};


template<class T>
struct struct_size
    : boost::mpl::size_t<
        boost::mpl::accumulate<
            typename struct_traits<T>::members,
            boost::mpl::size_t<0>,
            boost::mpl::plus<
                boost::mpl::_1,
                member_size<boost::mpl::_2>
            >
        >::type::value
    >
{
};


template<class T>
struct binary_size : detail::sizeof_impl<T> {};


template<class T>
struct member_offset
    : boost::mpl::size_t<
        boost::mpl::accumulate<
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
        >::type::value
    >
{
};

template<class Struct, class Type, Type Struct::* PtrToMember>
struct binary_offset : member_offset<member_base<Struct, Type, PtrToMember> >
{
};

} // End namespace hamigaki.

#endif // HAMIGAKI_BINARY_STRUCT_TRAITS_HPP
