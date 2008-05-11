// sha256.hpp: SHA-256 checksum

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#ifndef HAMIGAKI_CHECKSUM_SHA256_HPP
#define HAMIGAKI_CHECKSUM_SHA256_HPP

#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <cstddef>

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    #include <stdlib.h>
#endif

namespace hamigaki { namespace checksum {

namespace sha256_detail
{

typedef boost::uint32_t word;
typedef boost::array<word,16> block;

inline word rotate_right(word n, word s)
{
    BOOST_ASSERT(s != 0);
    BOOST_ASSERT(s < 32u);

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    return ::_rotr(n, static_cast<int>(s));
#elif defined(__MWERKS__) && (defined(__MC68K__) || defined(__INTEL__))
    return __ror(n, s);
#else
    return (n >> s) | (n << (32-s));
#endif
}

class sha256_impl
{
public:
    typedef boost::array<boost::uint8_t,32> value_type;

    sha256_impl()
    {
        reset();
    }

    void reset()
    {
        h_[0] = 0x6A09E667;
        h_[1] = 0xBB67AE85;
        h_[2] = 0x3C6EF372;
        h_[3] = 0xA54FF53A;
        h_[4] = 0x510E527F;
        h_[5] = 0x9B05688C;
        h_[6] = 0x1F83D9AB;
        h_[7] = 0x5BE0CD19;
    }

    void process_block(const block& x)
    {
        std::copy(x.begin(), x.end(), &w_[0]);

        for (word t = 16; t < 64; ++t)
            w_[t] = ssig1(w_[t-2]) + w_[t-7] + ssig0(w_[t-15]) + w_[t-16];

        word a = h_[0];
        word b = h_[1];
        word c = h_[2];
        word d = h_[3];
        word e = h_[4];
        word f = h_[5];
        word g = h_[6];
        word h = h_[7];

        for (word t = 0; t < 64; ++t)
        {
            word t1 = h + bsig1(e) + ch(e, f, g) + k(t) + w_[t];
            word t2 = bsig0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        h_[0] += a;
        h_[1] += b;
        h_[2] += c;
        h_[3] += d;
        h_[4] += e;
        h_[5] += f;
        h_[6] += g;
        h_[7] += h;
    }

    value_type output()
    {
        value_type tmp;
        tmp[ 0] = static_cast<boost::uint8_t>(h_[0] >> 24);
        tmp[ 1] = static_cast<boost::uint8_t>(h_[0] >> 16);
        tmp[ 2] = static_cast<boost::uint8_t>(h_[0] >>  8);
        tmp[ 3] = static_cast<boost::uint8_t>(h_[0]      );
        tmp[ 4] = static_cast<boost::uint8_t>(h_[1] >> 24);
        tmp[ 5] = static_cast<boost::uint8_t>(h_[1] >> 16);
        tmp[ 6] = static_cast<boost::uint8_t>(h_[1] >>  8);
        tmp[ 7] = static_cast<boost::uint8_t>(h_[1]      );
        tmp[ 8] = static_cast<boost::uint8_t>(h_[2] >> 24);
        tmp[ 9] = static_cast<boost::uint8_t>(h_[2] >> 16);
        tmp[10] = static_cast<boost::uint8_t>(h_[2] >>  8);
        tmp[11] = static_cast<boost::uint8_t>(h_[2]      );
        tmp[12] = static_cast<boost::uint8_t>(h_[3] >> 24);
        tmp[13] = static_cast<boost::uint8_t>(h_[3] >> 16);
        tmp[14] = static_cast<boost::uint8_t>(h_[3] >>  8);
        tmp[15] = static_cast<boost::uint8_t>(h_[3]      );
        tmp[16] = static_cast<boost::uint8_t>(h_[4] >> 24);
        tmp[17] = static_cast<boost::uint8_t>(h_[4] >> 16);
        tmp[18] = static_cast<boost::uint8_t>(h_[4] >>  8);
        tmp[19] = static_cast<boost::uint8_t>(h_[4]      );
        tmp[20] = static_cast<boost::uint8_t>(h_[5] >> 24);
        tmp[21] = static_cast<boost::uint8_t>(h_[5] >> 16);
        tmp[22] = static_cast<boost::uint8_t>(h_[5] >>  8);
        tmp[23] = static_cast<boost::uint8_t>(h_[5]      );
        tmp[24] = static_cast<boost::uint8_t>(h_[6] >> 24);
        tmp[25] = static_cast<boost::uint8_t>(h_[6] >> 16);
        tmp[26] = static_cast<boost::uint8_t>(h_[6] >>  8);
        tmp[27] = static_cast<boost::uint8_t>(h_[6]      );
        tmp[28] = static_cast<boost::uint8_t>(h_[7] >> 24);
        tmp[29] = static_cast<boost::uint8_t>(h_[7] >> 16);
        tmp[30] = static_cast<boost::uint8_t>(h_[7] >>  8);
        tmp[31] = static_cast<boost::uint8_t>(h_[7]      );
        return tmp;
    }

private:
    word w_[64];
    word h_[8];

    static word ch(word x, word y, word z)
    {
        return (x & y) ^ ((~x) & z);
    }

    static word maj(word x, word y, word z)
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    static word bsig0(word x)
    {
        return rotate_right(x, 2) ^ rotate_right(x, 13) ^ rotate_right(x, 22);
    }

    static word bsig1(word x)
    {
        return rotate_right(x, 6) ^ rotate_right(x, 11) ^ rotate_right(x, 25);
    }

    static word ssig0(word x)
    {
        return rotate_right(x, 7) ^ rotate_right(x, 18) ^ (x >> 3);
    }

    static word ssig1(word x)
    {
        return rotate_right(x, 17) ^ rotate_right(x, 19) ^ (x >> 10);
    }

    static word k(word t)
    {
        const word table[] =
        {
            0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
            0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
            0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
            0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
            0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
            0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
            0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
            0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
            0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
            0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
            0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
            0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
            0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
            0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
            0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
            0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
        };
        return table[t];
    }
};

} // namespace sha256_detail

class sha256
{
    typedef sha256_detail::word word;

public:
    typedef boost::array<boost::uint8_t,32> value_type;

    sha256()
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
        sha256 tmp(*this);

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
    sha256_detail::block buffer_;
    boost::uint64_t bit_;
    sha256_detail::sha256_impl impl_;
};

} } // End namespaces checksum, hamigaki.

#endif // HAMIGAKI_CHECKSUM_SHA256_HPP
