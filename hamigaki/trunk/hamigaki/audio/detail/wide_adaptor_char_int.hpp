// wide_adaptor_char_int.hpp: char <-> integer converter

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_WIDE_ADAPTOR_CHAR_INT_HPP
#define HAMIGAKI_AUDIO_DETAIL_WIDE_ADAPTOR_CHAR_INT_HPP

#include <hamigaki/audio/detail/cvt_int32.hpp>
#include <hamigaki/audio/detail/float.hpp>
#include <hamigaki/iostreams/positioning.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/integer.hpp>
#include <vector>

namespace hamigaki { namespace audio { namespace detail {

template<
    std::size_t Bits,
    class Device,
    class CharT = typename boost::int_t<Bits>::least
>
class wide_adaptor_char_int
{
    static const boost::int32_t slide_bits =
        8*(4 - ((Bits/8>=4) + (Bits/8>=3) + (Bits/8>=2) + (Bits/8>=1)));

public:
    typedef CharT char_type;

    wide_adaptor_char_int(const Device& dev, std::streamsize buffer_size)
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
            total += write_once(s + total, n - total);

        return total;
    }

    std::streampos seek(
        boost::iostreams::stream_offset off,
        BOOST_IOS::seekdir way, BOOST_IOS::openmode which)
    {
        const std::streamsize smp_sz = sample_size(type_);
        off *= smp_sz;
        std::streampos pos = boost::iostreams::seek(dev_, off, way, which);
        off = iostreams::to_offset(pos);
        off /= smp_sz;
        return iostreams::to_position(off);
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
        else if (type_ == mu_law)
            return read_int<mu_law>(s, n);
        else if (type_ == a_law)
            return read_int<a_law>(s, n);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported format");
        BOOST_UNREACHABLE_RETURN(-1)
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
        else if (type_ == mu_law)
            return write_int<mu_law>(s, n);
        else if (type_ == a_law)
            return write_int<a_law>(s, n);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported format");
        BOOST_UNREACHABLE_RETURN(-1)
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
                static_cast<boost::int32_t>(s[i]) << slide_bits);
        }

        boost::iostreams::write(dev_, &buffer_[0], count*smp_sz);

        return count;
    }

    template<endianness E, float_format Format>
    std::streamsize read_float(char_type* s, std::streamsize n)
    {
        typedef float_traits<Format> traits_type;

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
                hamigaki::decode_uint<E,sz>(&buffer_[offset]);

            float tmp24 =
                detail::decode_ieee754<float,Format>(tmp)*8388608;

            if (tmp24 >= 8388608)
                tmp24 = 8388607;
            else if (tmp24 < -8388608)
                tmp24 = -8388608;

            boost::int32_t val =
                static_cast<boost::int32_t>(tmp24) * 256;

            s[i] = static_cast<char_type>(val >> slide_bits);
        }

        return count;
    }

    template<endianness E, float_format Format>
    std::streamsize write_float(const char_type* s, std::streamsize n)
    {
        typedef float_traits<Format> traits_type;

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
            float val = static_cast<float>(
                (static_cast<boost::int32_t>(s[i]) << slide_bits)
                / 256) / 8388608;

            typename integer_encoding_traits<sz>::int_type tmp =
                detail::encode_ieee754<float,Format>(val);
            hamigaki::encode_uint<E,sz>(&buffer_[offset], tmp);
        }

        boost::iostreams::write(dev_, &buffer_[0], count*smp_sz);

        return count;
    }
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_WIDE_ADAPTOR_CHAR_INT_HPP
