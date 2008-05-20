// wide_adaptor_char_float.hpp: char <-> floating point converter

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_WIDE_ADAPTOR_CHAR_FLOAT_HPP
#define HAMIGAKI_AUDIO_DETAIL_WIDE_ADAPTOR_CHAR_FLOAT_HPP

#include <hamigaki/audio/detail/a_law.hpp>
#include <hamigaki/audio/detail/cvt_int32.hpp>
#include <hamigaki/audio/detail/float.hpp>
#include <hamigaki/audio/detail/mu_law.hpp>
#include <hamigaki/iostreams/positioning.hpp>
#include <boost/iostreams/operations.hpp>
#include <vector>

namespace hamigaki { namespace audio { namespace detail {

template<class CharT, class Device>
class wide_adaptor_char_float
{
public:
    typedef CharT char_type;

    wide_adaptor_char_float(const Device& dev, std::streamsize buffer_size)
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
        else if (type_ == mu_law)
            return read_mu_law(s, n);
        else if (type_ == a_law)
            return read_a_law(s, n);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported format");
        BOOST_UNREACHABLE_RETURN(-1)
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
            return write_float<little,ieee754_single>(s, n);
        else if (type_ == float_be32)
            return write_float<big,ieee754_single>(s, n);
        else if (type_ == float_le64)
            return write_float<little,ieee754_double>(s, n);
        else if (type_ == float_be64)
            return write_float<big,ieee754_double>(s, n);
        else if (type_ == mu_law)
            return write_mu_law(s, n);
        else if (type_ == a_law)
            return write_a_law(s, n);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported format");
        BOOST_UNREACHABLE_RETURN(-1)
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
            float tmp24 = s[i]*8388608;

            if (tmp24 >= 8388608)
                tmp24 = 8388607;
            else if (tmp24 < -8388608)
                tmp24 = -8388608;

            detail::cvt_int32<Type>::encode(
                &buffer_[offset],
                static_cast<boost::int32_t>(tmp24) * 256);
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
                hamigaki::decode_uint<E,sz>(&buffer_[offset]);
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
            hamigaki::encode_uint<E,sz>(&buffer_[offset], tmp);
        }

        boost::iostreams::write(dev_, &buffer_[0], count*smp_sz);

        return count;
    }

    std::streamsize read_mu_law(CharT* s, std::streamsize n)
    {
        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())
            );

        std::streamsize amt = boost::iostreams::read(dev_, &buffer_[0], count);
        if (amt == -1)
            return -1;
        count = amt;

        for (std::streamsize i = 0; i < count; ++i)
        {
            s[i] = detail::decode_mu_law<CharT>(
                static_cast<unsigned char>(buffer_[i])
            );
        }

        return count;
    }

    std::streamsize write_mu_law(const CharT* s, std::streamsize n)
    {
        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())
            );

        for (std::streamsize i = 0; i < count; ++i)
        {
            buffer_[i] = static_cast<char>(
                static_cast<unsigned char>(detail::encode_mu_law<CharT>(s[i]))
            );
        }

        boost::iostreams::write(dev_, &buffer_[0], count);

        return count;
    }

    std::streamsize read_a_law(CharT* s, std::streamsize n)
    {
        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())
            );

        std::streamsize amt = boost::iostreams::read(dev_, &buffer_[0], count);
        if (amt == -1)
            return -1;
        count = amt;

        for (std::streamsize i = 0; i < count; ++i)
        {
            s[i] = detail::decode_a_law<CharT>(
                static_cast<unsigned char>(buffer_[i])
            );
        }

        return count;
    }

    std::streamsize write_a_law(const CharT* s, std::streamsize n)
    {
        std::streamsize count =
            (std::min)(
                n,
                static_cast<std::streamsize>(buffer_.size())
            );

        for (std::streamsize i = 0; i < count; ++i)
        {
            buffer_[i] = static_cast<char>(
                static_cast<unsigned char>(detail::encode_a_law<CharT>(s[i]))
            );
        }

        boost::iostreams::write(dev_, &buffer_[0], count);

        return count;
    }
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_WIDE_ADAPTOR_CHAR_FLOAT_HPP
