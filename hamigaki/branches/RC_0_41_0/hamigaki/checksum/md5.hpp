// md5.hpp: MD5 checksum

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#ifndef HAMIGAKI_CHECKSUM_MD5_HPP
#define HAMIGAKI_CHECKSUM_MD5_HPP

#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <cstddef>

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    #include <stdlib.h>
#endif

namespace hamigaki { namespace checksum {

namespace md5_detail
{

typedef boost::uint32_t word;
typedef boost::array<word,16> block;

inline word rotate_left(word n, word s)
{
    BOOST_ASSERT(s != 0);
    BOOST_ASSERT(s < 32u);

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    return ::_rotl(n, static_cast<int>(s));
#elif defined(__MWERKS__) && (defined(__MC68K__) || defined(__INTEL__))
    return __rol(n, s);
#else
    return (n << s) | (n >> (32-s));
#endif
}

inline word func_f(word x, word y, word z)
{
    return (x & y) | (~x & z);
}

inline word func_g(word x, word y, word z)
{
    return (x & z) | (y & ~z);
}

inline word func_h(word x, word y, word z)
{
    return x ^ y ^ z;
}

inline word func_i(word x, word y, word z)
{
    return y ^ (x | ~z);
}

inline word round1_op(word a, word b, word c, word d, word x, word t, word s)
{
    return b + rotate_left(a + func_f(b,c,d) + x + t, s);
}

inline word round2_op(word a, word b, word c, word d, word x, word t, word s)
{
    return b + rotate_left(a + func_g(b,c,d) + x + t, s);
}

inline word round3_op(word a, word b, word c, word d, word x, word t, word s)
{
    return b + rotate_left(a + func_h(b,c,d) + x + t, s);
}

inline word round4_op(word a, word b, word c, word d, word x, word t, word s)
{
    return b + rotate_left(a + func_i(b,c,d) + x + t, s);
}

inline void round1(word& a, word& b, word& c, word& d, const block& x)
{
    a = round1_op(a, b, c, d, x[ 0], 0xD76AA478,  7);
    d = round1_op(d, a, b, c, x[ 1], 0xE8C7B756, 12);
    c = round1_op(c, d, a, b, x[ 2], 0x242070DB, 17);
    b = round1_op(b, c, d, a, x[ 3], 0xC1BDCEEE, 22);
    a = round1_op(a, b, c, d, x[ 4], 0xF57C0FAF,  7);
    d = round1_op(d, a, b, c, x[ 5], 0x4787C62A, 12);
    c = round1_op(c, d, a, b, x[ 6], 0xA8304613, 17);
    b = round1_op(b, c, d, a, x[ 7], 0xFD469501, 22);
    a = round1_op(a, b, c, d, x[ 8], 0x698098D8,  7);
    d = round1_op(d, a, b, c, x[ 9], 0x8B44F7AF, 12);
    c = round1_op(c, d, a, b, x[10], 0xFFFF5BB1, 17);
    b = round1_op(b, c, d, a, x[11], 0x895CD7BE, 22);
    a = round1_op(a, b, c, d, x[12], 0x6B901122,  7);
    d = round1_op(d, a, b, c, x[13], 0xFD987193, 12);
    c = round1_op(c, d, a, b, x[14], 0xA679438E, 17);
    b = round1_op(b, c, d, a, x[15], 0x49B40821, 22);
}

inline void round2(word& a, word& b, word& c, word& d, const block& x)
{
    a = round2_op(a, b, c, d, x[ 1], 0xF61E2562,  5);
    d = round2_op(d, a, b, c, x[ 6], 0xC040B340,  9);
    c = round2_op(c, d, a, b, x[11], 0x265E5A51, 14);
    b = round2_op(b, c, d, a, x[ 0], 0xE9B6C7AA, 20);
    a = round2_op(a, b, c, d, x[ 5], 0xD62F105D,  5);
    d = round2_op(d, a, b, c, x[10], 0x02441453,  9);
    c = round2_op(c, d, a, b, x[15], 0xD8A1E681, 14);
    b = round2_op(b, c, d, a, x[ 4], 0xE7D3FBC8, 20);
    a = round2_op(a, b, c, d, x[ 9], 0x21E1CDE6,  5);
    d = round2_op(d, a, b, c, x[14], 0xC33707D6,  9);
    c = round2_op(c, d, a, b, x[ 3], 0xF4D50D87, 14);
    b = round2_op(b, c, d, a, x[ 8], 0x455A14ED, 20);
    a = round2_op(a, b, c, d, x[13], 0xA9E3E905,  5);
    d = round2_op(d, a, b, c, x[ 2], 0xFCEFA3F8,  9);
    c = round2_op(c, d, a, b, x[ 7], 0x676F02D9, 14);
    b = round2_op(b, c, d, a, x[12], 0x8D2A4C8A, 20);
}

inline void round3(word& a, word& b, word& c, word& d, const block& x)
{
    a = round3_op(a, b, c, d, x[ 5], 0xFFFA3942,  4);
    d = round3_op(d, a, b, c, x[ 8], 0x8771F681, 11);
    c = round3_op(c, d, a, b, x[11], 0x6D9D6122, 16);
    b = round3_op(b, c, d, a, x[14], 0xFDE5380C, 23);
    a = round3_op(a, b, c, d, x[ 1], 0xA4BEEA44,  4);
    d = round3_op(d, a, b, c, x[ 4], 0x4BDECFA9, 11);
    c = round3_op(c, d, a, b, x[ 7], 0xF6BB4B60, 16);
    b = round3_op(b, c, d, a, x[10], 0xBEBFBC70, 23);
    a = round3_op(a, b, c, d, x[13], 0x289B7EC6,  4);
    d = round3_op(d, a, b, c, x[ 0], 0xEAA127FA, 11);
    c = round3_op(c, d, a, b, x[ 3], 0xD4EF3085, 16);
    b = round3_op(b, c, d, a, x[ 6], 0x04881D05, 23);
    a = round3_op(a, b, c, d, x[ 9], 0xD9D4D039,  4);
    d = round3_op(d, a, b, c, x[12], 0xE6DB99E5, 11);
    c = round3_op(c, d, a, b, x[15], 0x1FA27CF8, 16);
    b = round3_op(b, c, d, a, x[ 2], 0xC4AC5665, 23);
}

inline void round4(word& a, word& b, word& c, word& d, const block& x)
{
    a = round4_op(a, b, c, d, x[ 0], 0xF4292244,  6);
    d = round4_op(d, a, b, c, x[ 7], 0x432AFF97, 10);
    c = round4_op(c, d, a, b, x[14], 0xAB9423A7, 15);
    b = round4_op(b, c, d, a, x[ 5], 0xFC93A039, 21);
    a = round4_op(a, b, c, d, x[12], 0x655B59C3,  6);
    d = round4_op(d, a, b, c, x[ 3], 0x8F0CCC92, 10);
    c = round4_op(c, d, a, b, x[10], 0xFFEFF47D, 15);
    b = round4_op(b, c, d, a, x[ 1], 0x85845DD1, 21);
    a = round4_op(a, b, c, d, x[ 8], 0x6FA87E4F,  6);
    d = round4_op(d, a, b, c, x[15], 0xFE2CE6E0, 10);
    c = round4_op(c, d, a, b, x[ 6], 0xA3014314, 15);
    b = round4_op(b, c, d, a, x[13], 0x4E0811A1, 21);
    a = round4_op(a, b, c, d, x[ 4], 0xF7537E82,  6);
    d = round4_op(d, a, b, c, x[11], 0xBD3AF235, 10);
    c = round4_op(c, d, a, b, x[ 2], 0x2AD7D2BB, 15);
    b = round4_op(b, c, d, a, x[ 9], 0xEB86D391, 21);
}

class md5_impl
{
public:
    typedef boost::array<boost::uint8_t,16> value_type;

    md5_impl()
    {
        reset();
    }

    void reset()
    {
        a_ = 0x67452301;
        b_ = 0xEFCDAB89;
        c_ = 0x98BADCFE;
        d_ = 0x10325476;
    }

    void process_block(const block& x)
    {
        word a = a_;
        word b = b_;
        word c = c_;
        word d = d_;

        round1(a, b, c, d, x);
        round2(a, b, c, d, x);
        round3(a, b, c, d, x);
        round4(a, b, c, d, x);

        a_ += a;
        b_ += b;
        c_ += c;
        d_ += d;
    }

    value_type output()
    {
        value_type tmp;
        tmp[ 0] = static_cast<boost::uint8_t>(a_      );
        tmp[ 1] = static_cast<boost::uint8_t>(a_ >>  8);
        tmp[ 2] = static_cast<boost::uint8_t>(a_ >> 16);
        tmp[ 3] = static_cast<boost::uint8_t>(a_ >> 24);
        tmp[ 4] = static_cast<boost::uint8_t>(b_      );
        tmp[ 5] = static_cast<boost::uint8_t>(b_ >>  8);
        tmp[ 6] = static_cast<boost::uint8_t>(b_ >> 16);
        tmp[ 7] = static_cast<boost::uint8_t>(b_ >> 24);
        tmp[ 8] = static_cast<boost::uint8_t>(c_);
        tmp[ 9] = static_cast<boost::uint8_t>(c_ >>  8);
        tmp[10] = static_cast<boost::uint8_t>(c_ >> 16);
        tmp[11] = static_cast<boost::uint8_t>(c_ >> 24);
        tmp[12] = static_cast<boost::uint8_t>(d_);
        tmp[13] = static_cast<boost::uint8_t>(d_ >>  8);
        tmp[14] = static_cast<boost::uint8_t>(d_ >> 16);
        tmp[15] = static_cast<boost::uint8_t>(d_ >> 24);
        return tmp;
    }

private:
    word a_, b_, c_, d_;
};

} // namespace md5_detail

class md5
{
    typedef md5_detail::word word;

public:
    typedef boost::array<boost::uint8_t,16> value_type;

    md5()
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
        buffer_[index] |= static_cast<word>(bit) << (offset/8*8 + (7-offset%8));
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
        md5 tmp(*this);

        boost::uint64_t total = tmp.bit_;

        std::size_t pad_size =
            static_cast<std::size_t>((511 + 448 - bit_) % 512 + 1);

        tmp.process_bit(true);
        while (--pad_size)
            tmp.process_bit(false);

        for (int i = 0; i < 64; i += 8)
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
    md5_detail::block buffer_;
    boost::uint64_t bit_;
    md5_detail::md5_impl impl_;
};

} } // End namespaces checksum, hamigaki.

#endif // HAMIGAKI_CHECKSUM_MD5_HPP
