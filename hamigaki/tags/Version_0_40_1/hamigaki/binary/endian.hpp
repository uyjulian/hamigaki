// endian.hpp: little/big endian codec

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/binary

#ifndef HAMIGAKI_BINARY_ENDIAN_HPP
#define HAMIGAKI_BINARY_ENDIAN_HPP

#include <boost/detail/endian.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/cstdint.hpp>
#include <boost/integer.hpp>

namespace hamigaki {

enum endianness
{
    big, little, default_,
#if defined(BOOST_BIG_ENDIAN)
    native = big
#else
    native = little
#endif
};

template<std::size_t Size>
struct integer_encoding_traits;

template<>
struct integer_encoding_traits<1>
{
    typedef boost::int8_t int_type;
    typedef boost::uint8_t uint_type;
};

template<>
struct integer_encoding_traits<2>
{
    typedef boost::int16_t int_type;
    typedef boost::uint16_t uint_type;
};

template<>
struct integer_encoding_traits<3>
{
    typedef boost::int_t<24>::least int_type;
    typedef boost::uint_t<24>::least uint_type;
};

template<>
struct integer_encoding_traits<4>
{
    typedef boost::int32_t int_type;
    typedef boost::uint32_t uint_type;
};

template<>
struct integer_encoding_traits<8>
{
    typedef boost::int64_t int_type;
    typedef boost::uint64_t uint_type;
};

namespace detail
{

template<typename T, endianness E, std::size_t Size>
struct encode_uint_impl;

template<typename T>
struct encode_uint_impl<T,big,0>
{
    static void encode(char*, T)
    {
    }
};

template<typename T>
struct encode_uint_impl<T,little,0>
{
    static void encode(char*, T)
    {
    }
};

template<typename T, std::size_t Size>
struct encode_uint_impl<T,big,Size>
{
    static void encode(char* s, T n)
    {
        s[Size-1] = static_cast<char>(static_cast<unsigned char>(n & 0xFF));
        encode_uint_impl<T,big,Size-1>::encode(s, n >> 8);
    }
};

template<typename T, std::size_t Size>
struct encode_uint_impl<T,little,Size>
{
    static void encode(char* s, T n)
    {
        s[0] = static_cast<char>(static_cast<unsigned char>(n & 0xFF));
        encode_uint_impl<T,little,Size-1>::encode(s+1, n >> 8);
    }
};

template<>
struct encode_uint_impl<boost::uint8_t,big,1>
{
    static void encode(char* s, boost::uint8_t n)
    {
        *s = static_cast<unsigned char>(n);
    }
};

template<>
struct encode_uint_impl<boost::uint8_t,little,1>
{
    static void encode(char* s, boost::uint8_t n)
    {
        *s = static_cast<unsigned char>(n);
    }
};

#if defined(_M_IX86) || defined(__i386__)
template<>
struct encode_uint_impl<boost::uint16_t,native,2>
{
    static void encode(char* s, boost::uint16_t n)
    {
        *reinterpret_cast<boost::uint16_t*>(s) = n;
    }
};

template<>
struct encode_uint_impl<boost::uint32_t,native,4>
{
    static void encode(char* s, boost::uint32_t n)
    {
        *reinterpret_cast<boost::uint32_t*>(s) = n;
    }
};

template<>
struct encode_uint_impl<boost::uint64_t,native,8>
{
    static void encode(char* s, boost::uint64_t n)
    {
        *reinterpret_cast<boost::uint64_t*>(s) = n;
    }
};
#endif


template<typename T, endianness E, std::size_t Size>
struct decode_uint_impl;

template<typename T>
struct decode_uint_impl<T,big,0>
{
    static T decode(const char*)
    {
        return T();
    }
};

template<typename T>
struct decode_uint_impl<T,little,0>
{
    static T decode(const char*)
    {
        return T();
    }
};

template<typename T, std::size_t Size>
struct decode_uint_impl<T,big,Size>
{
    static T decode(const char* s)
    {
        return
            static_cast<T>(static_cast<unsigned char>(s[Size-1])) |
            (decode_uint_impl<T,big,Size-1>::decode(s) << 8);
    }
};

template<typename T, std::size_t Size>
struct decode_uint_impl<T,little,Size>
{
    static T decode(const char* s)
    {
        return
            static_cast<T>(static_cast<unsigned char>(s[0])) |
            (decode_uint_impl<T,little,Size-1>::decode(s+1) << 8);
    }
};

#if defined(_M_IX86) || defined(__i386__)
template<>
struct decode_uint_impl<boost::uint16_t,native,2>
{
    static boost::uint16_t decode(const char* s)
    {
        return *reinterpret_cast<const boost::uint16_t*>(s);
    }
};

template<>
struct decode_uint_impl<boost::uint32_t,native,4>
{
    static boost::uint32_t decode(const char* s)
    {
        return *reinterpret_cast<const boost::uint32_t*>(s);
    }
};

template<>
struct decode_uint_impl<boost::uint64_t,native,8>
{
    static boost::uint64_t decode(const char* s)
    {
        return *reinterpret_cast<const boost::uint64_t*>(s);
    }
};
#endif

template<typename T, std::size_t Size, class SameSize>
struct sign_extension_impl
{
    T operator()(T n) const
    {
        return n;
    }
};

template<typename T, std::size_t Size>
struct sign_extension_impl<T,Size,boost::mpl::false_>
{
    T operator()(T n) const
    {
        return static_cast<T>(n | (~static_cast<T>(0) << (Size*8)));
    }
};

template<std::size_t Size, typename T>
inline T sign_extension(T n)
{
    return sign_extension_impl<
        T,Size,boost::mpl::bool_<sizeof(T)==Size>
    >()(n);
}

} // namespace detail

template<endianness E, std::size_t Size>
inline void encode_uint(
    char* s, typename integer_encoding_traits<Size>::uint_type n)
{
    typedef typename integer_encoding_traits<Size>::uint_type uint_type;
    detail::encode_uint_impl<uint_type,E,Size>::encode(s, n);
}

template<endianness E, std::size_t Size>
inline void encode_int(
    char* s, typename integer_encoding_traits<Size>::int_type n)
{
    typedef typename integer_encoding_traits<Size>::uint_type uint_type;
    uint_type tmp = (n >= 0)
        ? static_cast<uint_type>(n)
        : ~static_cast<uint_type>(-(n + 1));
    detail::encode_uint_impl<uint_type,E,Size>::encode(s, tmp);
}

template<endianness E, std::size_t Size>
inline typename integer_encoding_traits<Size>::uint_type
decode_uint(const char* s)
{
    typedef typename integer_encoding_traits<Size>::uint_type uint_type;
    return detail::decode_uint_impl<uint_type,E,Size>::decode(s);
}

template<endianness E, std::size_t Size>
inline typename integer_encoding_traits<Size>::int_type
decode_int(const char* s)
{
    typedef typename integer_encoding_traits<Size>::int_type int_type;
    typedef typename integer_encoding_traits<Size>::uint_type uint_type;

    uint_type tmp = detail::decode_uint_impl<uint_type,E,Size>::decode(s);
    if ((tmp & (static_cast<uint_type>(1) << (8*Size - 1))) != 0)
        return -static_cast<int_type>(~detail::sign_extension<Size>(tmp)) - 1;
    else
        return static_cast<int_type>(tmp);
}

} // End namespace hamigaki.

#endif // HAMIGAKI_BINARY_ENDIAN_HPP
