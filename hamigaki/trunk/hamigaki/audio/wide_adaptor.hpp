//  wide_adaptor.hpp: an adaptor for making wide character stream

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_WIDE_ADAPTOR_HPP
#define HAMIGAKI_AUDIO_WIDE_ADAPTOR_HPP

#include <hamigaki/audio/detail/endian.hpp>
#include <hamigaki/audio/detail/float.hpp>
#include <hamigaki/audio/sample_format.hpp>
#include <hamigaki/iostreams/traits.hpp>
#include <boost/iostreams/detail/select.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/cstdint.hpp>
#include <boost/integer.hpp>
#include <vector>

namespace hamigaki { namespace audio {

namespace detail
{

template<sample_format_type Type>
struct cvt_int32;

template<> struct cvt_int32<uint8>
{
    static boost::int_least32_t decode(const char* s)
    {
        boost::int_least32_t tmp = static_cast<unsigned char>(*s);
        return (tmp - 128) * 16777216;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        boost::uint8_t tmp = static_cast<boost::uint8_t>(n/16777216 + 128);
        *s = static_cast<char>(static_cast<unsigned char>(tmp));
    }
};

template<> struct cvt_int32<int8>
{
    static boost::int_least32_t decode(const char* s)
    {
        boost::int_least32_t tmp = static_cast<signed char>(*s);
        return tmp * 16777216;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        boost::int8_t tmp = static_cast<boost::int8_t>(n/16777216);
        *s = static_cast<char>(static_cast<signed char>(tmp));
    }
};

template<> struct cvt_int32<int_le16>
{
    static boost::int_least32_t decode(const char* s)
    {
        boost::int_least32_t val = detail::decode_int<little,2>(s);
        return val * 65536;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        boost::int_least16_t val = static_cast<boost::int_least16_t>(n/65536);
        detail::encode_int<little,2>(s, val);
    }
};

template<> struct cvt_int32<int_be16>
{
    static boost::int_least32_t decode(const char* s)
    {
        boost::int_least32_t val = detail::decode_int<big,2>(s);
        return val * 65536;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        boost::int_least16_t val = static_cast<boost::int_least16_t>(n/65536);
        detail::encode_int<big,2>(s, val);
    }
};

template<> struct cvt_int32<int_le24>
{
    static boost::int_least32_t decode(const char* s)
    {
        boost::int_least32_t val = detail::decode_int<little,3>(s);
        return val * 256;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        typedef boost::int_t<24>::least int_type;
        int_type val = static_cast<int_type>(n/256);
        detail::encode_int<little,3>(s, val);
    }
};

template<> struct cvt_int32<int_be24>
{
    static boost::int_least32_t decode(const char* s)
    {
        boost::int_least32_t val = detail::decode_int<big,3>(s);
        return val * 256;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        typedef boost::int_t<24>::least int_type;
        int_type val = static_cast<int_type>(n/256);
        detail::encode_int<big,3>(s, val);
    }
};

template<> struct cvt_int32<int_le32>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<little,4>(s);
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<little,4>(s, n);
    }
};

template<> struct cvt_int32<int_be32>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<big,4>(s);
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<big,4>(s, n);
    }
};

template<> struct cvt_int32<int_a4_le16>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<little,4>(s) * 65536;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<little,4>(s, n/65536);
    }
};

template<> struct cvt_int32<int_a4_be16>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<big,4>(s) * 65536;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<big,4>(s, n/65536);
    }
};

template<> struct cvt_int32<int_a4_le18>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<little,4>(s) * 16384;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<little,4>(s, n/16384);
    }
};

template<> struct cvt_int32<int_a4_be18>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<big,4>(s) * 16384;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<big,4>(s, n/16384);
    }
};

template<> struct cvt_int32<int_a4_le20>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<little,4>(s) * 4096;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<little,4>(s, n/4096);
    }
};

template<> struct cvt_int32<int_a4_be20>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<big,4>(s) * 4096;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<big,4>(s, n/4096);
    }
};

template<> struct cvt_int32<int_a4_le24>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<little,4>(s) * 256;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<little,4>(s, n/256);
    }
};

template<> struct cvt_int32<int_a4_be24>
{
    static boost::int_least32_t decode(const char* s)
    {
        return detail::decode_int<big,4>(s) * 256;
    }

    static void encode(char* s, boost::int_least32_t n)
    {
        detail::encode_int<big,4>(s, n/256);
    }
};

template<class CharT, class Device>
class wide_adaptor_impl
{
public:
    typedef CharT char_type;

    wide_adaptor_impl(const Device& dev, std::streamsize buffer_size)
        : dev_(dev), buffer_(buffer_size)
        , type_(audio::sample_format_of(dev_))
    {
    }

    void close(BOOST_IOS::openmode which)
    { 
        boost::iostreams::close(dev_, which);
    }

    std::streamsize read(CharT* s, std::streamsize n)
    {
        std::streamsize total = 0;

        while (total != n)
        {
            std::streamsize amt = read_once(s + total, n - total);
            if (amt == -1)
                break;
            total += amt;
        }

        return (total != 0) ? total : -1;
    }

    std::streamsize write(const CharT* s, std::streamsize n)
    {
        std::streamsize total = 0;

        while (total != n)
        {
            std::streamsize amt = write_once(s + total, n - total);
            if (amt == -1)
                break;
            total += amt;
        }

        return (total != 0) ? total : -1;
    }

    std::streamsize optimal_buffer_size() const
    {
        return buffer_.size() / sample_size(type_);
    }

private:
    Device dev_;
    std::vector<char> buffer_;
    sample_format_type type_;

    std::streamsize read_once(CharT* s, std::streamsize n)
    {
        if (type_ == uint8)
            return read_int<uint8>(s, n);
        else if (type_ == int8)
            return read_int<int8>(s, n);
        else if (type_ == int_le16)
            return read_int<int_le16>(s, n);
        else if (type_ == int_be16)
            return read_int<int_be16>(s, n);
        else if (type_ == int_le24)
            return read_int<int_le24>(s, n);
        else if (type_ == int_be24)
            return read_int<int_be24>(s, n);
        else if (type_ == int_le32)
            return read_int<int_le32>(s, n);
        else if (type_ == int_be32)
            return read_int<int_be32>(s, n);
        else if (type_ == int_a4_le16)
            return read_int<int_a4_le16>(s, n);
        else if (type_ == int_a4_be16)
            return read_int<int_a4_be16>(s, n);
        else if (type_ == int_a4_le18)
            return read_int<int_a4_le18>(s, n);
        else if (type_ == int_a4_be18)
            return read_int<int_a4_be18>(s, n);
        else if (type_ == int_a4_le20)
            return read_int<int_a4_le20>(s, n);
        else if (type_ == int_a4_be20)
            return read_int<int_a4_be20>(s, n);
        else if (type_ == int_a4_le24)
            return read_int<int_a4_le24>(s, n);
        else if (type_ == int_a4_be24)
            return read_int<int_a4_be24>(s, n);
        else if (type_ == float_le32)
            return read_float<little,ieee754_single>(s, n);
        else if (type_ == float_be32)
            return read_float<big,ieee754_single>(s, n);
        else if (type_ == float_le64)
            return read_float<little,ieee754_double>(s, n);
        else if (type_ == float_be64)
            return read_float<big,ieee754_double>(s, n);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported format");
        return -1; // dummy
    }

    std::streamsize write_once(const CharT* s, std::streamsize n)
    {
        if (type_ == uint8)
            return write_int<uint8>(s, n);
        else if (type_ == int8)
            return write_int<int8>(s, n);
        else if (type_ == int_le16)
            return write_int<int_le16>(s, n);
        else if (type_ == int_be16)
            return write_int<int_be16>(s, n);
        else if (type_ == int_le24)
            return write_int<int_le24>(s, n);
        else if (type_ == int_be24)
            return write_int<int_be24>(s, n);
        else if (type_ == int_le32)
            return write_int<int_le32>(s, n);
        else if (type_ == int_be32)
            return write_int<int_be32>(s, n);
        else if (type_ == int_a4_le16)
            return write_int<int_a4_le16>(s, n);
        else if (type_ == int_a4_be16)
            return write_int<int_a4_be16>(s, n);
        else if (type_ == int_a4_le18)
            return write_int<int_a4_le18>(s, n);
        else if (type_ == int_a4_be18)
            return write_int<int_a4_be18>(s, n);
        else if (type_ == int_a4_le20)
            return write_int<int_a4_le20>(s, n);
        else if (type_ == int_a4_be20)
            return write_int<int_a4_be20>(s, n);
        else if (type_ == int_a4_le24)
            return write_int<int_a4_le24>(s, n);
        else if (type_ == int_a4_be24)
            return write_int<int_a4_be24>(s, n);
        else if (type_ == float_le32)
            return write_int<int_le32>(s, n);
        else if (type_ == float_be32)
            return write_float<big,ieee754_single>(s, n);
        else if (type_ == float_be32)
            return write_float<big,ieee754_single>(s, n);
        else if (type_ == float_le64)
            return write_float<little,ieee754_double>(s, n);
        else if (type_ == float_be64)
            return write_float<big,ieee754_double>(s, n);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported format");
        return -1; // dummy
    }

    template<sample_format_type Type>
    std::streamsize read_int(CharT* s, std::streamsize n)
    {
        const std::streamsize smp_sz = sample_size(Type);

        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())/smp_sz
            );

        std::streamsize amt =
            boost::iostreams::read(dev_, &buffer_[0], count*smp_sz);
        if (amt == -1)
            return -1;
        count = amt / smp_sz;

        for (std::streamsize i = 0, offset = 0;
            i < count; ++i, offset += smp_sz)
        {
            s[i] = static_cast<CharT>(
                detail::cvt_int32<Type>::decode(
                    &buffer_[offset]) / 256) / static_cast<CharT>(8388608);
        }

        return count;
    }

    template<sample_format_type Type>
    std::streamsize write_int(const CharT* s, std::streamsize n)
    {
        const std::streamsize smp_sz = sample_size(Type);

        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())/smp_sz
            );

        for (std::streamsize i = 0, offset = 0;
            i < count; ++i, offset += smp_sz)
        {
            typedef typename float_t<24>::least float_type;
            float_type tmp24 = s[i]*8388608;

            if (tmp24 >= 8388608)
                tmp24 = 8388607;
            else if (tmp24 < -8388608)
                tmp24 = -8388608;

            detail::cvt_int32<Type>::encode(
                &buffer_[offset],
                static_cast<boost::int_least32_t>(tmp24) * 256);
        }

        boost::iostreams::write(dev_, &buffer_[0], count*smp_sz);

        return count;
    }

    template<endianness E, float_format Format>
    std::streamsize read_float(CharT* s, std::streamsize n)
    {
        const std::size_t sz = float_traits<Format>::bits / 8;
        const std::streamsize smp_sz = static_cast<std::streamsize>(sz);

        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())/smp_sz
            );

        std::streamsize amt =
            boost::iostreams::read(dev_, &buffer_[0], count*smp_sz);
        if (amt == -1)
            return -1;
        count = amt / smp_sz;

        for (std::streamsize i = 0, offset = 0;
            i < count; ++i, offset += smp_sz)
        {
            typename integer_encoding_traits<sz>::int_type tmp =
                detail::decode_uint<E,sz>(&buffer_[offset]);
            s[i] = detail::decode_ieee754<CharT,Format>(tmp);
        }

        return count;
    }

    template<endianness E, float_format Format>
    std::streamsize write_float(const CharT* s, std::streamsize n)
    {
        const std::size_t sz = float_traits<Format>::bits / 8;
        const std::streamsize smp_sz = static_cast<std::streamsize>(sz);

        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())/smp_sz
            );

        for (std::streamsize i = 0, offset = 0;
            i < count; ++i, offset += smp_sz)
        {
            typename integer_encoding_traits<sz>::int_type tmp =
                detail::encode_ieee754<CharT,Format>(s[i]);
            detail::encode_uint<E,sz>(&buffer_[offset], tmp);
        }

        boost::iostreams::write(dev_, &buffer_[0], count*smp_sz);

        return count;
    }
};

template<
    std::size_t Bits,
    class Device,
    class CharT = typename boost::int_t<Bits>::least
>
class wide_adaptor_int
{
    static const boost::int_least32_t slide_bits =
        8*(4 - ((Bits/8>=4) + (Bits/8>=3) + (Bits/8>=2) + (Bits/8>=1)));

public:
    typedef CharT char_type;

    wide_adaptor_int(const Device& dev, std::streamsize buffer_size)
        : dev_(dev), buffer_(buffer_size)
        , type_(audio::sample_format_of(dev_))
    {
    }

    void close(BOOST_IOS::openmode which)
    { 
        boost::iostreams::close(dev_, which);
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        std::streamsize total = 0;

        while (total != n)
        {
            std::streamsize amt = read_once(s + total, n - total);
            if (amt == -1)
                break;
            total += amt;
        }

        return (total != 0) ? total : -1;
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        std::streamsize total = 0;

        while (total != n)
        {
            std::streamsize amt = write_once(s + total, n - total);
            if (amt == -1)
                break;
            total += amt;
        }

        return (total != 0) ? total : -1;
    }

    std::streamsize optimal_buffer_size() const
    {
        return buffer_.size() / sample_size(type_);
    }

private:
    Device dev_;
    std::vector<char> buffer_;
    sample_format_type type_;

    std::streamsize read_once(char_type* s, std::streamsize n)
    {
        if (type_ == uint8)
            return read_int<uint8>(s, n);
        else if (type_ == int8)
            return read_int<int8>(s, n);
        else if (type_ == int_le16)
            return read_int<int_le16>(s, n);
        else if (type_ == int_be16)
            return read_int<int_be16>(s, n);
        else if (type_ == int_le24)
            return read_int<int_le24>(s, n);
        else if (type_ == int_be24)
            return read_int<int_be24>(s, n);
        else if (type_ == int_le32)
            return read_int<int_le32>(s, n);
        else if (type_ == int_be32)
            return read_int<int_be32>(s, n);
        else if (type_ == int_a4_le16)
            return read_int<int_a4_le16>(s, n);
        else if (type_ == int_a4_be16)
            return read_int<int_a4_be16>(s, n);
        else if (type_ == int_a4_le18)
            return read_int<int_a4_le18>(s, n);
        else if (type_ == int_a4_be18)
            return read_int<int_a4_be18>(s, n);
        else if (type_ == int_a4_le20)
            return read_int<int_a4_le20>(s, n);
        else if (type_ == int_a4_be20)
            return read_int<int_a4_be20>(s, n);
        else if (type_ == int_a4_le24)
            return read_int<int_a4_le24>(s, n);
        else if (type_ == int_a4_be24)
            return read_int<int_a4_be24>(s, n);
        else if (type_ == float_le32)
            return read_float<little,ieee754_single>(s, n);
        else if (type_ == float_be32)
            return read_float<big,ieee754_single>(s, n);
        else if (type_ == float_le64)
            return read_float<little,ieee754_double>(s, n);
        else if (type_ == float_be64)
            return read_float<big,ieee754_double>(s, n);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported format");
        return -1; // dummy
    }

    std::streamsize write_once(const char_type* s, std::streamsize n)
    {
        if (type_ == uint8)
            return write_int<uint8>(s, n);
        else if (type_ == int8)
            return write_int<int8>(s, n);
        else if (type_ == int_le16)
            return write_int<int_le16>(s, n);
        else if (type_ == int_be16)
            return write_int<int_be16>(s, n);
        else if (type_ == int_le24)
            return write_int<int_le24>(s, n);
        else if (type_ == int_be24)
            return write_int<int_be24>(s, n);
        else if (type_ == int_le32)
            return write_int<int_le32>(s, n);
        else if (type_ == int_be32)
            return write_int<int_be32>(s, n);
        else if (type_ == int_a4_le16)
            return write_int<int_a4_le16>(s, n);
        else if (type_ == int_a4_be16)
            return write_int<int_a4_be16>(s, n);
        else if (type_ == int_a4_le18)
            return write_int<int_a4_le18>(s, n);
        else if (type_ == int_a4_be18)
            return write_int<int_a4_be18>(s, n);
        else if (type_ == int_a4_le20)
            return write_int<int_a4_le20>(s, n);
        else if (type_ == int_a4_be20)
            return write_int<int_a4_be20>(s, n);
        else if (type_ == int_a4_le24)
            return write_int<int_a4_le24>(s, n);
        else if (type_ == int_a4_be24)
            return write_int<int_a4_be24>(s, n);
        else if (type_ == float_le32)
            return write_int<int_le32>(s, n);
        else if (type_ == float_be32)
            return write_float<big,ieee754_single>(s, n);
        else if (type_ == float_be32)
            return write_float<big,ieee754_single>(s, n);
        else if (type_ == float_le64)
            return write_float<little,ieee754_double>(s, n);
        else if (type_ == float_be64)
            return write_float<big,ieee754_double>(s, n);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported format");
        return -1; // dummy
    }

    template<sample_format_type Type>
    std::streamsize read_int(char_type* s, std::streamsize n)
    {
        const std::streamsize smp_sz = sample_size(Type);

        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())/smp_sz
            );

        std::streamsize amt =
            boost::iostreams::read(dev_, &buffer_[0], count*smp_sz);
        if (amt == -1)
            return -1;
        count = amt / smp_sz;

        for (std::streamsize i = 0, offset = 0;
            i < count; ++i, offset += smp_sz)
        {
            s[i] = static_cast<char_type>(detail::cvt_int32<Type>
                ::decode(&buffer_[offset]) >> slide_bits);
        }

        return count;
    }

    template<sample_format_type Type>
    std::streamsize write_int(const char_type* s, std::streamsize n)
    {
        const std::streamsize smp_sz = sample_size(Type);

        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())/smp_sz
            );

        for (std::streamsize i = 0, offset = 0;
            i < count; ++i, offset += smp_sz)
        {
            detail::cvt_int32<Type>::encode(
                &buffer_[offset],
                static_cast<boost::int_least32_t>(s[i]) << slide_bits);
        }

        boost::iostreams::write(dev_, &buffer_[0], count*smp_sz);

        return count;
    }

    template<endianness E, float_format Format>
    std::streamsize read_float(char_type* s, std::streamsize n)
    {
        typedef float_traits<Format> traits_type;
        typedef typename float_t<24>::least float_type;

        const std::size_t sz = traits_type::bits / 8;
        const std::streamsize smp_sz = static_cast<std::streamsize>(sz);

        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())/smp_sz
            );

        std::streamsize amt =
            boost::iostreams::read(dev_, &buffer_[0], count*smp_sz);
        if (amt == -1)
            return -1;
        count = amt / smp_sz;

        for (std::streamsize i = 0, offset = 0;
            i < count; ++i, offset += smp_sz)
        {
            typename integer_encoding_traits<sz>::int_type tmp =
                detail::decode_uint<E,sz>(&buffer_[offset]);

            float_type tmp24 =
                detail::decode_ieee754<float_type,Format>(tmp)*8388608;

            if (tmp24 >= 8388608)
                tmp24 = 8388607;
            else if (tmp24 < -8388608)
                tmp24 = -8388608;

            boost::int_least32_t val =
                static_cast<boost::int_least32_t>(tmp24) * 256;

            s[i] = static_cast<char_type>(val >> slide_bits);
        }

        return count;
    }

    template<endianness E, float_format Format>
    std::streamsize write_float(const char_type* s, std::streamsize n)
    {
        typedef float_traits<Format> traits_type;
        typedef typename float_t<24>::least float_type;

        const std::size_t sz = traits_type::bits / 8;
        const std::streamsize smp_sz = static_cast<std::streamsize>(sz);

        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())/smp_sz
            );

        for (std::streamsize i = 0, offset = 0;
            i < count; ++i, offset += smp_sz)
        {
            float_type val = static_cast<float_type>(
                (static_cast<boost::int_least32_t>(s[i]) << slide_bits)
                / 256) / 8388608;

            typename integer_encoding_traits<sz>::int_type tmp =
                detail::encode_ieee754<float_type,Format>(val);
            detail::encode_uint<E,sz>(&buffer_[offset], tmp);
        }

        boost::iostreams::write(dev_, &buffer_[0], count*smp_sz);

        return count;
    }
};

template<class CharT, class Device>
class wide_adaptor_float
{
    typedef typename boost::iostreams::
        char_type_of<Device>::type base_char_type;

public:
    typedef CharT char_type;

    wide_adaptor_float(const Device& dev, std::streamsize buffer_size)
        : dev_(dev), buffer_(buffer_size)
    {
    }

    void close(BOOST_IOS::openmode which)
    { 
        boost::iostreams::close(dev_, which);
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        std::streamsize total = 0;

        while (total != n)
        {
            std::streamsize amt = read_once(s + total, n - total);
            if (amt == -1)
                break;
            total += amt;
        }

        return (total != 0) ? total : -1;
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        std::streamsize total = 0;

        while (total != n)
        {
            std::streamsize amt = write_once(s + total, n - total);
            if (amt == -1)
                break;
            total += amt;
        }

        return (total != 0) ? total : -1;
    }

    std::streamsize optimal_buffer_size() const
    {
        return buffer_.size();
    }

private:
    Device dev_;
    std::vector<base_char_type> buffer_;

    std::streamsize read_once(char_type* s, std::streamsize n)
    {
        std::streamsize count =
            (std::min)(n, static_cast<std::streamsize>(buffer_.size()));

        std::streamsize amt =
            boost::iostreams::read(dev_, &buffer_[0], count);
        if (amt == -1)
            return -1;

        for (std::streamsize i = 0; i < amt; ++i)
            s[i] = static_cast<char_type>(buffer_[i]);

        return amt;
    }

    std::streamsize write_once(const char_type* s, std::streamsize n)
    {
        std::streamsize count =
            (std::min)(n, static_cast<std::streamsize>(buffer_.size()));

        for (std::streamsize i = 0; i < count; ++i)
            buffer_[i] = static_cast<base_char_type>(s[i]);

        return boost::iostreams::write(dev_, &buffer_[0], count);
    }
};

template<class CharT, std::size_t Bits>
struct is_int_exact_t
{
    static const bool value =
        std::numeric_limits<CharT>::is_integer &&
        std::numeric_limits<CharT>::is_signed &&
        std::numeric_limits<CharT>::digits+1 == Bits;
};

} // namespace detail

template<class CharT, class Device>
class wide_adaptor
{
    typedef typename boost::iostreams::
        char_type_of<Device>::type base_char_type;

    typedef typename
        boost::iostreams::select<
            boost::is_floating_point<base_char_type>,
                detail::wide_adaptor_float<CharT,Device>,

            boost::is_same<CharT,boost::int_t<32> >,
                detail::wide_adaptor_int<32,Device>,
            boost::is_same<CharT,boost::int_t<24> >,
                detail::wide_adaptor_int<24,Device>,
            boost::is_same<CharT,boost::int_t<16> >,
                detail::wide_adaptor_int<16,Device>,

            detail::is_int_exact_t<CharT,32>,
                detail::wide_adaptor_int<32,Device,CharT>,
            detail::is_int_exact_t<CharT,24>,
                detail::wide_adaptor_int<24,Device,CharT>,
            detail::is_int_exact_t<CharT,16>,
                detail::wide_adaptor_int<16,Device,CharT>,

            boost::iostreams::else_,
                detail::wide_adaptor_impl<CharT,Device>
        >::type impl_type;

public:
    typedef typename impl_type::char_type char_type;

    struct category :
        boost::iostreams::mode_of<Device>::type,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        boost::iostreams::optimally_buffered_tag {};

    explicit wide_adaptor(const Device& dev)
        : pimpl_(
            new impl_type(dev, boost::iostreams::optimal_buffer_size(dev)))
    {
    }

    wide_adaptor(const Device& dev, std::streamsize buffer_size)
        : pimpl_(new impl_type(dev, buffer_size))
    {
    }

    void close()
    {
        this->close_impl(
            boost::is_convertible<
                typename boost::iostreams::mode_of<Device>::type,
                boost::iostreams::output
            >()
        );
    }

    void close(BOOST_IOS::openmode which)
    {
        pimpl_->close(which);
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    std::streamsize optimal_buffer_size() const
    {
        return pimpl_->optimal_buffer_size();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;

    void close_impl(const boost::false_type&)
    {
        pimpl_->close(BOOST_IOS::in);
    }

    void close_impl(const boost::true_type&)
    {
        pimpl_->close(BOOST_IOS::out);
    }
};

template<class CharT, class Device>
inline wide_adaptor<CharT, Device>
widen(const Device& dev)
{
    return wide_adaptor<CharT, Device>(dev);
}

template<class CharT, class Device>
inline wide_adaptor<CharT, Device>
widen(const Device& dev, std::streamsize buffer_size)
{
    return wide_adaptor<CharT, Device>(dev, buffer_size);
}

} } // End namespaces audio, hamigaki.

#endif // HAMIGAKI_AUDIO_WIDE_ADAPTOR_HPP
