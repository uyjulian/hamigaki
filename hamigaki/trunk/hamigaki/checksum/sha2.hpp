// sha2.hpp: SHA-2 checksum

// Copyright Takeshi Mouri 2008, 2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#ifndef HAMIGAKI_CHECKSUM_SHA2_HPP
#define HAMIGAKI_CHECKSUM_SHA2_HPP

#include <hamigaki/integer/byte_swap.hpp>
#include <hamigaki/integer/rotate.hpp>
#include <boost/detail/endian.hpp>
#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <cstddef>

namespace hamigaki { namespace checksum {

namespace sha2_detail
{

inline void to_big32(boost::uint8_t* dst, boost::uint32_t n)
{
#if defined(BOOST_BIG_ENDIAN)
    std::memcpy(dst, &n, sizeof(n));
#elif defined(_M_IX86) || defined(__i386__)
    *reinterpret_cast<boost::uint32_t*>(dst) = byte_swap32(n);
#else
    dst[0] = static_cast<boost::uint8_t>(n >> 24);
    dst[1] = static_cast<boost::uint8_t>(n >> 16);
    dst[2] = static_cast<boost::uint8_t>(n >>  8);
    dst[3] = static_cast<boost::uint8_t>(n      );
#endif
}

inline void to_big64(boost::uint8_t* dst, boost::uint64_t n)
{
#if defined(BOOST_BIG_ENDIAN)
    std::memcpy(dst, &n, sizeof(n));
#elif defined(_M_IX86) || defined(__i386__)
    *reinterpret_cast<boost::uint32_t*>(dst) =
        byte_swap32(static_cast<boost::uint32_t>(n >> 32));
    *reinterpret_cast<boost::uint32_t*>(dst + 4) =
        byte_swap32(static_cast<boost::uint32_t>(n));
#else
    dst[0] = static_cast<boost::uint8_t>(n >> 56);
    dst[1] = static_cast<boost::uint8_t>(n >> 48);
    dst[2] = static_cast<boost::uint8_t>(n >> 40);
    dst[3] = static_cast<boost::uint8_t>(n >> 32);
    dst[4] = static_cast<boost::uint8_t>(n >> 24);
    dst[5] = static_cast<boost::uint8_t>(n >> 16);
    dst[6] = static_cast<boost::uint8_t>(n >>  8);
    dst[7] = static_cast<boost::uint8_t>(n      );
#endif
}

template<std::size_t Size>
struct sha2_traits;

template<>
struct sha2_traits<224>
{
    typedef boost::uint32_t word;
    typedef boost::array<boost::uint8_t,28> value_type;

    static void reset(word (&h)[8])
    {
        h[0] = 0xC1059ED8;
        h[1] = 0x367CD507;
        h[2] = 0x3070DD17;
        h[3] = 0xF70E5939;
        h[4] = 0xFFC00B31;
        h[5] = 0x68581511;
        h[6] = 0x64F98FA7;
        h[7] = 0xBEFA4FA4;
    }

    static value_type output(const word (&h)[8])
    {
        value_type tmp;
        to_big32(&tmp.elems[ 0], h[0]);
        to_big32(&tmp.elems[ 4], h[1]);
        to_big32(&tmp.elems[ 8], h[2]);
        to_big32(&tmp.elems[12], h[3]);
        to_big32(&tmp.elems[16], h[4]);
        to_big32(&tmp.elems[20], h[5]);
        to_big32(&tmp.elems[24], h[6]);
        return tmp;
    }
};

template<>
struct sha2_traits<256>
{
    typedef boost::uint32_t word;
    typedef boost::array<boost::uint8_t,32> value_type;

    static void reset(word (&h)[8])
    {
        h[0] = 0x6A09E667;
        h[1] = 0xBB67AE85;
        h[2] = 0x3C6EF372;
        h[3] = 0xA54FF53A;
        h[4] = 0x510E527F;
        h[5] = 0x9B05688C;
        h[6] = 0x1F83D9AB;
        h[7] = 0x5BE0CD19;
    }

    static value_type output(const word (&h)[8])
    {
        value_type tmp;
        to_big32(&tmp.elems[ 0], h[0]);
        to_big32(&tmp.elems[ 4], h[1]);
        to_big32(&tmp.elems[ 8], h[2]);
        to_big32(&tmp.elems[12], h[3]);
        to_big32(&tmp.elems[16], h[4]);
        to_big32(&tmp.elems[20], h[5]);
        to_big32(&tmp.elems[24], h[6]);
        to_big32(&tmp.elems[28], h[7]);
        return tmp;
    }
};

template<>
struct sha2_traits<384>
{
    typedef boost::uint64_t word;
    typedef boost::array<boost::uint8_t,48> value_type;

    static void reset(word (&h)[8])
    {
        h[0] = 0xCBBB9D5DC1059ED8ull;
        h[1] = 0x629A292A367CD507ull;
        h[2] = 0x9159015A3070DD17ull;
        h[3] = 0x152FECD8F70E5939ull;
        h[4] = 0x67332667FFC00B31ull;
        h[5] = 0x8EB44A8768581511ull;
        h[6] = 0xDB0C2E0D64F98FA7ull;
        h[7] = 0x47B5481DBEFA4FA4ull;
    }

    static value_type output(const word (&h)[8])
    {
        value_type tmp;
        to_big64(&tmp.elems[ 0], h[0]);
        to_big64(&tmp.elems[ 8], h[1]);
        to_big64(&tmp.elems[16], h[2]);
        to_big64(&tmp.elems[24], h[3]);
        to_big64(&tmp.elems[32], h[4]);
        to_big64(&tmp.elems[40], h[5]);
        return tmp;
    }
};

template<>
struct sha2_traits<512>
{
    typedef boost::uint64_t word;
    typedef boost::array<boost::uint8_t,64> value_type;

    static void reset(word (&h)[8])
    {
        h[0] = 0x6A09E667F3BCC908ull;
        h[1] = 0xBB67AE8584CAA73Bull;
        h[2] = 0x3C6EF372FE94F82Bull;
        h[3] = 0xA54FF53A5F1D36F1ull;
        h[4] = 0x510E527FADE682D1ull;
        h[5] = 0x9B05688C2B3E6C1Full;
        h[6] = 0x1F83D9ABFB41BD6Bull;
        h[7] = 0x5BE0CD19137E2179ull;
    }

    static value_type output(const word (&h)[8])
    {
        value_type tmp;
        to_big64(&tmp.elems[ 0], h[0]);
        to_big64(&tmp.elems[ 8], h[1]);
        to_big64(&tmp.elems[16], h[2]);
        to_big64(&tmp.elems[24], h[3]);
        to_big64(&tmp.elems[32], h[4]);
        to_big64(&tmp.elems[40], h[5]);
        to_big64(&tmp.elems[48], h[6]);
        to_big64(&tmp.elems[56], h[7]);
        return tmp;
    }
};

template<class Word>
struct sha2_word_traits;

template<>
struct sha2_word_traits<boost::uint32_t>
{
    typedef boost::uint32_t word;
    static const std::size_t rounds = 64;

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
        static const word table[] =
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

template<>
struct sha2_word_traits<boost::uint64_t>
{
    typedef boost::uint64_t word;
    static const std::size_t rounds = 80;

    static word rotate_right(word x, word n)
    {
        return (x >> n) | (x << (64-n));
    }

    static word bsig0(word x)
    {
        return rotate_right(x, 28) ^ rotate_right(x, 34) ^ rotate_right(x, 39);
    }

    static word bsig1(word x)
    {
        return rotate_right(x, 14) ^ rotate_right(x, 18) ^ rotate_right(x, 41);
    }

    static word ssig0(word x)
    {
        return rotate_right(x, 1) ^ rotate_right(x, 8) ^ (x >> 7);
    }

    static word ssig1(word x)
    {
        return rotate_right(x, 19) ^ rotate_right(x, 61) ^ (x >> 6);
    }

    static word k(word t)
    {
        static const word table[] =
        {
            0x428A2F98D728AE22ull, 0x7137449123EF65CDull,
            0xB5C0FBCFEC4D3B2Full, 0xE9B5DBA58189DBBCull,
            0x3956C25BF348B538ull, 0x59F111F1B605D019ull,
            0x923F82A4AF194F9Bull, 0xAB1C5ED5DA6D8118ull,
            0xD807AA98A3030242ull, 0x12835B0145706FBEull,
            0x243185BE4EE4B28Cull, 0x550C7DC3D5FFB4E2ull,
            0x72BE5D74F27B896Full, 0x80DEB1FE3B1696B1ull,
            0x9BDC06A725C71235ull, 0xC19BF174CF692694ull,
            0xE49B69C19EF14AD2ull, 0xEFBE4786384F25E3ull,
            0x0FC19DC68B8CD5B5ull, 0x240CA1CC77AC9C65ull,
            0x2DE92C6F592B0275ull, 0x4A7484AA6EA6E483ull,
            0x5CB0A9DCBD41FBD4ull, 0x76F988DA831153B5ull,
            0x983E5152EE66DFABull, 0xA831C66D2DB43210ull,
            0xB00327C898FB213Full, 0xBF597FC7BEEF0EE4ull,
            0xC6E00BF33DA88FC2ull, 0xD5A79147930AA725ull,
            0x06CA6351E003826Full, 0x142929670A0E6E70ull,
            0x27B70A8546D22FFCull, 0x2E1B21385C26C926ull,
            0x4D2C6DFC5AC42AEDull, 0x53380D139D95B3DFull,
            0x650A73548BAF63DEull, 0x766A0ABB3C77B2A8ull,
            0x81C2C92E47EDAEE6ull, 0x92722C851482353Bull,
            0xA2BFE8A14CF10364ull, 0xA81A664BBC423001ull,
            0xC24B8B70D0F89791ull, 0xC76C51A30654BE30ull,
            0xD192E819D6EF5218ull, 0xD69906245565A910ull,
            0xF40E35855771202Aull, 0x106AA07032BBD1B8ull,
            0x19A4C116B8D2D0C8ull, 0x1E376C085141AB53ull,
            0x2748774CDF8EEB99ull, 0x34B0BCB5E19B48A8ull,
            0x391C0CB3C5C95A63ull, 0x4ED8AA4AE3418ACBull,
            0x5B9CCA4F7763E373ull, 0x682E6FF3D6B2B8A3ull,
            0x748F82EE5DEFB2FCull, 0x78A5636F43172F60ull,
            0x84C87814A1F0AB72ull, 0x8CC702081A6439ECull,
            0x90BEFFFA23631E28ull, 0xA4506CEBDE82BDE9ull,
            0xBEF9A3F7B2C67915ull, 0xC67178F2E372532Bull,
            0xCA273ECEEA26619Cull, 0xD186B8C721C0C207ull,
            0xEADA7DD6CDE0EB1Eull, 0xF57D4F7FEE6ED178ull,
            0x06F067AA72176FBAull, 0x0A637DC5A2C898A6ull,
            0x113F9804BEF90DAEull, 0x1B710B35131C471Bull,
            0x28DB77F523047D84ull, 0x32CAAB7B40C72493ull,
            0x3C9EBE0A15C9BEBCull, 0x431D67C49C100D4Cull,
            0x4CC5D4BECB3E42B6ull, 0x597F299CFC657E2Aull,
            0x5FCB6FAB3AD6FAECull, 0x6C44198C4A475817ull
        };
        return table[t];
    }
};


template<class Traits>
class sha2_impl
{
public:
    typedef typename Traits::value_type value_type;
    typedef typename Traits::word word;
    typedef boost::array<word,16> block;
    typedef sha2_word_traits<word> word_traits;
    static const std::size_t rounds = word_traits::rounds;

    sha2_impl()
    {
        reset();
    }

    void reset()
    {
        Traits::reset(h_);
    }

    void process_block(const block& x)
    {
        word w[rounds];
        std::copy(x.begin(), x.end(), &w[0]);

        for (word t = 16; t < rounds; ++t)
            w[t] = ssig1(w[t-2]) + w[t-7] + ssig0(w[t-15]) + w[t-16];

        word a = h_[0];
        word b = h_[1];
        word c = h_[2];
        word d = h_[3];
        word e = h_[4];
        word f = h_[5];
        word g = h_[6];
        word h = h_[7];

        for (word t = 0; t < rounds; ++t)
        {
            word t1 = h + bsig1(e) + ch(e, f, g) + k(t) + w[t];
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
        return Traits::output(h_);
    }

private:
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
        return word_traits::bsig0(x);
    }

    static word bsig1(word x)
    {
        return word_traits::bsig1(x);
    }

    static word ssig0(word x)
    {
        return word_traits::ssig0(x);
    }

    static word ssig1(word x)
    {
        return word_traits::ssig1(x);
    }

    static word k(word t)
    {
        return word_traits::k(t);
    }
};

} // namespace sha2_detail

template<std::size_t Size>
class sha2_basic
{
    typedef sha2_detail::sha2_traits<Size> traits_type;
    typedef sha2_detail::sha2_impl<traits_type> impl_type;
    typedef typename traits_type::word word;
    typedef typename impl_type::block block;

public:
    typedef typename impl_type::value_type value_type;

    static const std::size_t word_bits = sizeof(word) * 8;
    static const std::size_t block_bits = word_bits * block::static_size;

    sha2_basic()
    {
        reset();
    }

    void reset()
    {
        buffer_.assign(0);
        bits_high_ = 0;
        bits_low_ = 0;
        impl_.reset();
    }

    void process_bit(bool bit)
    {
        std::size_t index =
            static_cast<std::size_t>((bits_low_ % block_bits) / word_bits);
        std::size_t offset = static_cast<std::size_t>(bits_low_ % word_bits);
        buffer_[index] |= static_cast<word>(bit) << (word_bits-1 - offset);
        if (++bits_low_ == 0)
            ++bits_high_;
        if ((bits_low_ % block_bits) == 0)
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
        sha2_basic tmp(*this);

        word total_high = tmp.bits_high_;
        word total_low = tmp.bits_low_;

        std::size_t pad_size =
            static_cast<std::size_t>(
                block_bits - (tmp.bits_low_ + word_bits*2)%block_bits
            );

        tmp.process_bit(true);
        while (--pad_size)
            tmp.process_bit(false);

        for (int i = word_bits-8; i >= 0; i -= 8)
            tmp.process_byte(static_cast<unsigned char>(total_high >> i));

        for (int i = word_bits-8; i >= 0; i -= 8)
            tmp.process_byte(static_cast<unsigned char>(total_low >> i));

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
    block buffer_;
    word bits_high_;
    word bits_low_;
    impl_type impl_;
};

template<std::size_t Size>
class sha2_optimal
{
    typedef sha2_detail::sha2_traits<Size> traits_type;
    typedef sha2_detail::sha2_impl<traits_type> impl_type;
    typedef typename traits_type::word word;
    typedef typename impl_type::block block;

public:
    typedef typename impl_type::value_type value_type;

    static const std::size_t word_bytes = sizeof(word);
    static const std::size_t block_bytes = word_bytes * block::static_size;
    static const std::size_t word_bits = sizeof(word) * 8;
    static const std::size_t block_bits = word_bits * block::static_size;

    sha2_optimal()
    {
        reset();
    }

    void reset()
    {
        buffer_.assign(0);
        bytes_high_ = 0;
        bytes_low_ = 0;
        impl_.reset();
    }

    void process_byte(unsigned char byte)
    {
        std::size_t index =
            static_cast<std::size_t>((bytes_low_ % block_bytes) / word_bytes);
        std::size_t offset =
            static_cast<std::size_t>(bytes_low_ % word_bytes) * 8;
        buffer_[index] |= static_cast<word>(byte) << (word_bits-8 - offset);

        if (++bytes_low_ == 0)
            ++bytes_high_;

        if ((bytes_low_ % block_bytes) == 0)
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
        std::size_t index =
            static_cast<std::size_t>((bytes_low_ % block_bytes) / word_bytes);
        std::size_t offset =
            word_bits-8 - static_cast<std::size_t>(bytes_low_ % word_bytes) * 8;

        word old_low = bytes_low_;
        bytes_low_ += static_cast<word>(end - beg);
        if (bytes_low_ < old_low)
            ++bytes_high_;

        while (beg != end)
        {
            word b = *(beg++);
            buffer_[index] |= b << offset;
            if (offset == 0)
            {
                offset = word_bits-8;
                if (++index == block::static_size)
                {
                    index = 0;
                    impl_.process_block(buffer_);
                    buffer_.assign(0);
                }
            }
            else
                offset -= 8;
        }
    }

    void process_bytes(const void* buffer, std::size_t byte_count)
    {
        typedef unsigned char uchar;
        const uchar* beg = static_cast<const uchar*>(buffer);
        process_block(beg, beg+byte_count);
    }

    value_type checksum()
    {
        word total_high = (bytes_high_ << 3) | (bytes_low_ >> (word_bits-3));
        word total_low = bytes_low_ << 3;

        process_byte(0x80);
        if (bytes_low_ % block_bytes >= block_bytes-word_bytes*2)
        {
            impl_.process_block(buffer_);
            buffer_.assign(0);
        }

        buffer_[14] = total_high;
        buffer_[15] = total_low;
        impl_.process_block(buffer_);

        return impl_.output();
    }

    void operator()(unsigned char byte)
    {
        process_byte(byte);
    }

    value_type operator()()
    {
        return checksum();
    }

private:
    block buffer_;
    word bytes_high_;
    word bytes_low_;
    impl_type impl_;
};

typedef sha2_optimal<224> sha224;
typedef sha2_optimal<256> sha256;
typedef sha2_optimal<384> sha384;
typedef sha2_optimal<512> sha512;

} } // End namespaces checksum, hamigaki.

#endif // HAMIGAKI_CHECKSUM_SHA2_HPP
