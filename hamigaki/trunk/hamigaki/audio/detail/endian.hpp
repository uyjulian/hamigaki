//  endian.hpp: little/big endian codec

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_AUDIO_DETAIL_ENDIAN_HPP
#define HAMIGAKI_AUDIO_DETAIL_ENDIAN_HPP

#include <boost/cstdint.hpp>
#include <boost/integer.hpp>

namespace hamigaki { namespace audio { namespace detail {

enum endianness
{
    big, little
};

template<std::size_t Size>
struct integer_encoding_traits;

template<>
struct integer_encoding_traits<1>
{
    typedef boost::int_least8_t int_type;
    typedef boost::uint_least8_t uint_type;
};

template<>
struct integer_encoding_traits<2>
{
    typedef boost::int_least16_t int_type;
    typedef boost::uint_least16_t uint_type;
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
    typedef boost::int_least32_t int_type;
    typedef boost::uint_least32_t uint_type;
};

template<>
struct integer_encoding_traits<8>
{
    typedef boost::int_least64_t int_type;
    typedef boost::uint_least64_t uint_type;
};

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

template<endianness E, std::size_t Size>
inline void encode_uint(
    char* s, typename integer_encoding_traits<Size>::uint_type n)
{
    typedef typename integer_encoding_traits<Size>::uint_type uint_type;
    encode_uint_impl<uint_type,E,Size>::encode(s, n);
}

template<endianness E, std::size_t Size>
inline void encode_int(
    char* s, typename integer_encoding_traits<Size>::int_type n)
{
    typedef typename integer_encoding_traits<Size>::uint_type uint_type;
    uint_type tmp = (n >= 0)
        ? static_cast<uint_type>(n)
        : ~static_cast<uint_type>(-(n + 1));
    encode_uint_impl<uint_type,E,Size>::encode(s, tmp);
}


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

template<endianness E, std::size_t Size>
inline typename integer_encoding_traits<Size>::uint_type
decode_uint(const char* s)
{
    typedef typename integer_encoding_traits<Size>::uint_type uint_type;
    return decode_uint_impl<uint_type,E,Size>::decode(s);
}

template<endianness E, std::size_t Size>
inline typename integer_encoding_traits<Size>::int_type
decode_int(const char* s)
{
    typedef typename integer_encoding_traits<Size>::int_type int_type;
    typedef typename integer_encoding_traits<Size>::uint_type uint_type;

    uint_type tmp = decode_uint_impl<uint_type,E,Size>::decode(s);
    if ((tmp & (static_cast<uint_type>(1) << (8*Size - 1))) != 0)
        return -static_cast<int_type>(~tmp) - 1;
    else
        return static_cast<int_type>(tmp);
}

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_ENDIAN_HPP
