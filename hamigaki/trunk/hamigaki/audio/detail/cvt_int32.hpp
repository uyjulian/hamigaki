// cvt_int32.hpp: integer <-> int32_t converter

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_CVT_INT32_HPP
#define HAMIGAKI_AUDIO_DETAIL_CVT_INT32_HPP

#include <hamigaki/audio/sample_format.hpp>
#include <hamigaki/binary/endian.hpp>
#include <boost/assert.hpp>
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

template<> struct cvt_int32<mu_law>
{
private:
    static boost::uint16_t decode_impl(boost::uint8_t x)
    {
        unsigned high = (x >> 4);
        boost::uint16_t low = static_cast<boost::uint16_t>(x & 0x0F);

        switch (high)
        {
            default:
                BOOST_ASSERT(0);
            case 0x0:
                return low << 1;
            case 0x1:
                return (low << 2) + (31 + 2);
            case 0x2:
                return (low << 3) + (95 + 4);
            case 0x3:
                return (low << 4) + (223 + 8);
            case 0x4:
                return (low << 5) + (479 + 16);
            case 0x5:
                return (low << 6) + (991 + 32);
            case 0x6:
                return (low << 7) + (2015 + 64);
            case 0x7:
                return (low << 8) + (4063 + 128);
        }
    }

    static boost::uint8_t encode_impl(boost::uint16_t x)
    {
        if (x < 31)
            return 0x00 | ((x+1) >> 1);
        else if (x < 95)
            return 0x10 | ((x-31) >> 2);
        else if (x < 223)
            return 0x20 | ((x-95) >> 3);
        else if (x < 479)
            return 0x30 | ((x-223) >> 4);
        else if (x < 991)
            return 0x40 | ((x-479) >> 5);
        else if (x < 2015)
            return 0x50 | ((x-991) >> 6);
        else if (x < 4063)
            return 0x60 | ((x-2015) >> 7);
        else if (x < 8159)
            return 0x70 | ((x-4063) >> 8);
        else
            return 0x7F;
    }

public:
    static boost::int32_t decode(const char* s)
    {
        boost::uint8_t x = static_cast<unsigned char>(*s);
        if ((x & 0x80) == 0)
            return -static_cast<boost::int32_t>(decode_impl(0x7F-x) << 18);
        else
            return static_cast<boost::int32_t>(decode_impl(~x) << 18);
    }

    static void encode(char* s, boost::int32_t n)
    {
        if (n < 0)
        {
            boost::uint32_t x = static_cast<boost::uint32_t>(-(n+1)) >> 18;
            boost::uint8_t y = encode_impl(static_cast<boost::uint16_t>(x));
            *s = static_cast<char>(static_cast<unsigned char>(0x7F - y));
        }
        else
        {
            boost::uint32_t x = static_cast<boost::uint32_t>(n) >> 18;
            boost::uint8_t y = encode_impl(static_cast<boost::uint16_t>(x));
            *s = static_cast<char>(static_cast<unsigned char>(~y));
        }
    }
};

template<> struct cvt_int32<a_law>
{
private:
    static boost::uint16_t decode_impl(boost::uint8_t x)
    {
        unsigned high = (x >> 4);
        boost::uint16_t low = static_cast<boost::uint16_t>(x & 0x0F);

        if (high == 0)
            return (low << 1) | 1;
        else
            return ((low << 1) | 0x21) << (high-1);
    }

    static boost::uint8_t encode_impl(boost::uint16_t x)
    {
        if (x < 64)
            return x >> 1;
        else if (x < 128)
            return 0x20 | (((x & 0x07F) >> 2) & 0x0F);
        else if (x < 256)
            return 0x30 | (((x & 0x0FF) >> 3) & 0x0F);
        else if (x < 512)
            return 0x40 | (((x & 0x1FF) >> 4) & 0x0F);
        else if (x < 1024)
            return 0x50 | (((x & 0x3FF) >> 5) & 0x0F);
        else if (x < 2048)
            return 0x60 | (((x & 0x7FF) >> 6) & 0x0F);
        else if (x < 4096)
            return 0x70 | (((x & 0xFFF) >> 7) & 0x0F);
        else
            return 0x7F;
    }

public:
    static boost::int32_t decode(const char* s)
    {
        boost::uint8_t x = static_cast<unsigned char>(*s) ^ 0x55;
        if ((x & 0x80) == 0)
            return -static_cast<boost::int32_t>(decode_impl(x) << 19);
        else
            return static_cast<boost::int32_t>(decode_impl(x & 0x7F) << 19);
    }

    static void encode(char* s, boost::int32_t n)
    {
        if (n < 0)
        {
            boost::uint32_t x = static_cast<boost::uint32_t>(-(n+1)) >> 19;
            boost::uint8_t y = encode_impl(static_cast<boost::uint16_t>(x));
            *s = static_cast<char>(static_cast<unsigned char>(y ^ 0x55));
        }
        else
        {
            boost::uint32_t x = static_cast<boost::uint32_t>(n) >> 19;
            boost::uint8_t y = encode_impl(static_cast<boost::uint16_t>(x));
            *s = static_cast<char>(static_cast<unsigned char>((y|0x80) ^ 0x55));
        }
    }
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_CVT_INT32_HPP
