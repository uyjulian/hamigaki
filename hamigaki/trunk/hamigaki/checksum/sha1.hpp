// sha1.hpp: SHA-1 checksum

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#ifndef HAMIGAKI_CHECKSUM_SHA1_HPP
#define HAMIGAKI_CHECKSUM_SHA1_HPP

#include <hamigaki/integer/byte_swap.hpp>
#include <hamigaki/integer/rotate.hpp>
#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <cstddef>

namespace hamigaki { namespace checksum {

namespace sha1_detail
{

typedef boost::uint32_t word;
typedef boost::array<word,16> block;

inline void copy_word(boost::uint8_t* dst, word n)
{
#if defined(BOOST_BIG_ENDIAN)
    std::memcpy(dst, &n, sizeof(word));
#elif defined(_M_IX86) || defined(__i386__)
    *reinterpret_cast<word*>(dst) = byte_swap32(n);
#else
    dst[0] = static_cast<boost::uint8_t>(n >> 24);
    dst[1] = static_cast<boost::uint8_t>(n >> 16);
    dst[2] = static_cast<boost::uint8_t>(n >>  8);
    dst[3] = static_cast<boost::uint8_t>(n      );
#endif
}

class sha1_impl
{
public:
    typedef boost::array<boost::uint8_t,20> value_type;

    sha1_impl()
    {
        reset();
    }

    void reset()
    {
        h_[0] = 0x67452301;
        h_[1] = 0xEFCDAB89;
        h_[2] = 0x98BADCFE;
        h_[3] = 0x10325476;
        h_[4] = 0xC3D2E1F0;
    }

    void process_block(const block& x)
    {
        std::copy(x.begin(), x.end(), &w_[0]);

        for (word t = 16; t < 80; ++t)
            w_[t] = step_b(t);

        word a = h_[0];
        word b = h_[1];
        word c = h_[2];
        word d = h_[3];
        word e = h_[4];

        for (word t = 0; t < 80; ++t)
        {
            word temp = step_d(t, a, b, c, d, e);
            e = d;
            d = c;
            c = rotate_left(b, 30);
            b = a;
            a = temp;
        }

        h_[0] += a;
        h_[1] += b;
        h_[2] += c;
        h_[3] += d;
        h_[4] += e;
    }

    value_type output()
    {
        value_type tmp;
        copy_word(&tmp.elems[ 0], h_[0]);
        copy_word(&tmp.elems[ 4], h_[1]);
        copy_word(&tmp.elems[ 8], h_[2]);
        copy_word(&tmp.elems[12], h_[3]);
        copy_word(&tmp.elems[16], h_[4]);
        return tmp;
    }

private:
    word w_[80];
    word h_[5];

    word step_b(word t)
    {
        return rotate_left(w_[t-3] ^ w_[t-8] ^ w_[t-14] ^ w_[t-16], 1);
    }

    static word f(word t, word b, word c, word d)
    {
        if (t < 20)
            return (b & c) | (~b & d);
        else if (t < 40)
            return b ^ c ^ d;
        else if (t < 60)
            return (b & c) | (b & d) | (c & d);
        else
            return b ^ c ^ d;
    }

    static word k(word t)
    {
        if (t < 20)
            return 0x5A827999;
        else if (t < 40)
            return 0x6ED9EBA1;
        else if (t < 60)
            return 0x8F1BBCDC;
        else
            return 0xCA62C1D6;
    }

    word step_d(word t, word a, word b, word c, word d, word e)
    {
        return rotate_left(a, 5) + f(t, b, c, d) + e + w_[t] + k(t);
    }
};

} // namespace sha1_detail

class sha1
{
    typedef sha1_detail::word word;

public:
    typedef boost::array<boost::uint8_t,20> value_type;

    sha1()
    {
        reset();
    }

    void reset()
    {
        buffer_.assign(0);
        bit_ = 0;
        impl_.reset();
    }

    void process_bit(bool bit)
    {
        std::size_t index = static_cast<std::size_t>((bit_ % 512) / 32);
        std::size_t offset = static_cast<std::size_t>(bit_ % 32);
        buffer_[index] |= static_cast<word>(bit) << (31 - offset);
        if ((++bit_ % 512) == 0)
        {
            impl_.process_block(buffer_);
            buffer_.assign(0);
        }
    }

    void process_bits(unsigned char bits, std::size_t bit_count)
    {
        while (bit_count--)
            process_bit(((bits >> bit_count) & 1) != 0);
    }

    void process_byte(unsigned char byte)
    {
        process_bits(byte, 8);
    }

    void process_block(const void* bytes_begin, const void* bytes_end)
    {
        typedef unsigned char uchar;
        const uchar* beg = static_cast<const uchar*>(bytes_begin);
        const uchar* end = static_cast<const uchar*>(bytes_end);
        while (beg != end)
            process_byte(*(beg++));
    }

    void process_bytes(const void* buffer, std::size_t byte_count)
    {
        typedef unsigned char uchar;
        const uchar* beg = static_cast<const uchar*>(buffer);
        process_block(beg, beg+byte_count);
    }

    value_type checksum() const
    {
        sha1 tmp(*this);

        boost::uint64_t total = tmp.bit_;

        std::size_t pad_size =
            static_cast<std::size_t>(512 - (tmp.bit_ + 64)%512);

        tmp.process_bit(true);
        while (--pad_size)
            tmp.process_bit(false);

        for (int i = 56; i >= 0; i -= 8)
            tmp.process_byte(static_cast<unsigned char>(total >> i));

        return tmp.impl_.output();
    }

    void operator()(unsigned char byte)
    {
        process_byte(byte);
    }

    value_type operator()() const
    {
        return checksum();
    }

private:
    sha1_detail::block buffer_;
    boost::uint64_t bit_;
    sha1_detail::sha1_impl impl_;
};

class sha1_optimal
{
    typedef sha1_detail::word word;

public:
    typedef boost::array<boost::uint8_t,20> value_type;

    sha1_optimal()
    {
        reset();
    }

    void reset()
    {
        buffer_.assign(0);
        byte_ = 0;
        impl_.reset();
    }

    void process_byte(unsigned char byte)
    {
        std::size_t index = static_cast<std::size_t>((byte_ % 64) / 4);
        std::size_t offset = static_cast<std::size_t>(byte_ % 4) * 8;
        buffer_[index] |= byte << (24 - offset);
        if ((++byte_ % 64) == 0)
        {
            impl_.process_block(buffer_);
            buffer_.assign(0);
        }
    }

    void process_block(const void* bytes_begin, const void* bytes_end)
    {
        typedef unsigned char uchar;
        const uchar* beg = static_cast<const uchar*>(bytes_begin);
        const uchar* end = static_cast<const uchar*>(bytes_end);
        while (beg != end)
            process_byte(*(beg++));
    }

    void process_bytes(const void* buffer, std::size_t byte_count)
    {
        typedef unsigned char uchar;
        const uchar* beg = static_cast<const uchar*>(buffer);
        process_block(beg, beg+byte_count);
    }

    value_type checksum() const
    {
        sha1_optimal tmp(*this);

        boost::uint64_t total = tmp.byte_ * 8;

        std::size_t pad_size =
            static_cast<std::size_t>(64 - (tmp.byte_ + 8)%64);

        tmp.process_byte(0x80);
        while (--pad_size)
            tmp.process_byte(0x00);

        tmp.process_byte(static_cast<boost::uint8_t>(total >> 56));
        tmp.process_byte(static_cast<boost::uint8_t>(total >> 48));
        tmp.process_byte(static_cast<boost::uint8_t>(total >> 40));
        tmp.process_byte(static_cast<boost::uint8_t>(total >> 32));
        tmp.process_byte(static_cast<boost::uint8_t>(total >> 24));
        tmp.process_byte(static_cast<boost::uint8_t>(total >> 16));
        tmp.process_byte(static_cast<boost::uint8_t>(total >>  8));
        tmp.process_byte(static_cast<boost::uint8_t>(total      ));

        return tmp.impl_.output();
    }

    void operator()(unsigned char byte)
    {
        process_byte(byte);
    }

    value_type operator()() const
    {
        return checksum();
    }

private:
    sha1_detail::block buffer_;
    boost::uint64_t byte_;
    sha1_detail::sha1_impl impl_;
};

} } // End namespaces checksum, hamigaki.

#endif // HAMIGAKI_CHECKSUM_SHA1_HPP
