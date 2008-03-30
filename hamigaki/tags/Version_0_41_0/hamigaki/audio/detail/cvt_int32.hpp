// cvt_int32.hpp: integer <-> int32_t converter

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_CVT_INT32_HPP
#define HAMIGAKI_AUDIO_DETAIL_CVT_INT32_HPP

#include <hamigaki/audio/sample_format.hpp>
#include <hamigaki/binary/endian.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace audio { namespace detail {

template<sample_format_type Type>
struct cvt_int32;

template<> struct cvt_int32<uint8>
{
    static boost::int32_t decode(const char* s)
    {
        boost::int32_t tmp = static_cast<unsigned char>(*s);
        return (tmp - 128) * 16777216;
    }

    static void encode(char* s, boost::int32_t n)
    {
        boost::uint8_t tmp = static_cast<boost::uint8_t>(n/16777216 + 128);
        *s = static_cast<char>(static_cast<unsigned char>(tmp));
    }
};

template<> struct cvt_int32<int8>
{
    static boost::int32_t decode(const char* s)
    {
        boost::int32_t tmp = static_cast<signed char>(*s);
        return tmp * 16777216;
    }

    static void encode(char* s, boost::int32_t n)
    {
        boost::int8_t tmp = static_cast<boost::int8_t>(n/16777216);
        *s = static_cast<char>(static_cast<signed char>(tmp));
    }
};

template<> struct cvt_int32<int_le16>
{
    static boost::int32_t decode(const char* s)
    {
        boost::int32_t val = hamigaki::decode_int<little,2>(s);
        return val * 65536;
    }

    static void encode(char* s, boost::int32_t n)
    {
        boost::int16_t val = static_cast<boost::int16_t>(n/65536);
        hamigaki::encode_int<little,2>(s, val);
    }
};

template<> struct cvt_int32<int_be16>
{
    static boost::int32_t decode(const char* s)
    {
        boost::int32_t val = hamigaki::decode_int<big,2>(s);
        return val * 65536;
    }

    static void encode(char* s, boost::int32_t n)
    {
        boost::int16_t val = static_cast<boost::int16_t>(n/65536);
        hamigaki::encode_int<big,2>(s, val);
    }
};

template<> struct cvt_int32<int_le24>
{
    static boost::int32_t decode(const char* s)
    {
        boost::int32_t val = hamigaki::decode_int<little,3>(s);
        return val * 256;
    }

    static void encode(char* s, boost::int32_t n)
    {
        typedef boost::int_t<24>::least int_type;
        int_type val = static_cast<int_type>(n/256);
        hamigaki::encode_int<little,3>(s, val);
    }
};

template<> struct cvt_int32<int_be24>
{
    static boost::int32_t decode(const char* s)
    {
        boost::int32_t val = hamigaki::decode_int<big,3>(s);
        return val * 256;
    }

    static void encode(char* s, boost::int32_t n)
    {
        typedef boost::int_t<24>::least int_type;
        int_type val = static_cast<int_type>(n/256);
        hamigaki::encode_int<big,3>(s, val);
    }
};

template<> struct cvt_int32<int_le32>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<little,4>(s);
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<little,4>(s, n);
    }
};

template<> struct cvt_int32<int_be32>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<big,4>(s);
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<big,4>(s, n);
    }
};

template<> struct cvt_int32<int_a4_le16>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<little,4>(s) * 65536;
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<little,4>(s, n/65536);
    }
};

template<> struct cvt_int32<int_a4_be16>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<big,4>(s) * 65536;
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<big,4>(s, n/65536);
    }
};

template<> struct cvt_int32<int_a4_le18>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<little,4>(s) * 16384;
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<little,4>(s, n/16384);
    }
};

template<> struct cvt_int32<int_a4_be18>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<big,4>(s) * 16384;
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<big,4>(s, n/16384);
    }
};

template<> struct cvt_int32<int_a4_le20>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<little,4>(s) * 4096;
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<little,4>(s, n/4096);
    }
};

template<> struct cvt_int32<int_a4_be20>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<big,4>(s) * 4096;
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<big,4>(s, n/4096);
    }
};

template<> struct cvt_int32<int_a4_le24>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<little,4>(s) * 256;
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<little,4>(s, n/256);
    }
};

template<> struct cvt_int32<int_a4_be24>
{
    static boost::int32_t decode(const char* s)
    {
        return hamigaki::decode_int<big,4>(s) * 256;
    }

    static void encode(char* s, boost::int32_t n)
    {
        hamigaki::encode_int<big,4>(s, n/256);
    }
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_CVT_INT32_HPP
