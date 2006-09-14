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
};


template<class T>
struct member_size;

template<class Struct, class Type, Type Struct::* PtrToMember, endianness E>
struct member_size<member<Struct, Type, PtrToMember, E> >
{
    typedef boost::mpl::size_t<sizeof(Type)> type;
};


template<class T>
struct struct_traits;


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
