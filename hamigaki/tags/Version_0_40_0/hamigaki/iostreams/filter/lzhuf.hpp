// lzhuf.hpp: LZHUF compression/decompression

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_FILTER_LZHUF_HPP
#define HAMIGAKI_IOSTREAMS_FILTER_LZHUF_HPP

#include <hamigaki/iostreams/filter/modified_lzss.hpp>
#include <hamigaki/iostreams/filter/sliding_window.hpp>
#include <hamigaki/iostreams/utility/huffman.hpp>
#include <hamigaki/iostreams/bit_stream.hpp>
#include <boost/iostreams/detail/adapter/direct_adapter.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/assert.hpp>

namespace hamigaki { namespace iostreams {

namespace lha_detail
{

template<class InputBitStream>
inline boost::uint16_t decode_code_length(InputBitStream& bs)
{
    boost::uint16_t n = bs.read_bits(3);
    if (n == 7)
    {
        while (bs.get_bit())
            ++n;
    }
    return n;
}

template<class OutputBitStream>
inline void encode_code_length(OutputBitStream& bs, boost::uint16_t n)
{
    if (n < 7)
        bs.write_bits(n, 3);
    else
    {
        bs.write_bits(0x07, 3);
        n -= 7;
        while (n--)
            bs.put_bit(true);
        bs.put_bit(false);
    }
}

class lzhuf_input
{
public:
    typedef boost::uint16_t length_type;
    typedef boost::uint16_t offset_type;
    typedef literal_or_reference<offset_type,length_type> result_type;

    static const length_type min_match_length = 3;

    explicit lzhuf_input(std::size_t window_bits) : count_(0)
    {
        if (window_bits <= 13)
            offset_count_bits_ = 4;
        else
            offset_count_bits_ = 5;
    }

    template<class Source>
    result_type get(Source& src)
    {
        input_bit_stream<left_to_right, Source> bs(filter_, src);
        if (count_ == 0)
        {
            count_ = static_cast<boost::uint16_t>(bs.read_bits(16));
            decode_symbol_huffman_tree(bs);
            decode_offset_size_huffman_tree(bs);
        }

        --count_;
        boost::uint16_t symbol = symbol_decoder_.decode(bs);
        if (symbol < 256)
        {
            char literal =
                static_cast<char>(static_cast<unsigned char>(symbol));
            return result_type(literal);
        }
        else
        {
            boost::uint16_t length = symbol - 256 + min_match_length;
            boost::uint16_t size = offset_decoder_.decode(bs);

            boost::uint16_t offset = size;
            if (size > 1)
                offset = (1 << (size-1)) | bs.read_bits(size-1);

            return result_type(offset, length);
        }
    }

private:
    std::size_t count_;
    std::size_t offset_count_bits_;
    input_bit_filter<left_to_right> filter_;
    huffman_decoder<boost::uint16_t,16> symbol_decoder_;
    huffman_decoder<boost::uint16_t,16> offset_decoder_;
    huffman_decoder<boost::uint16_t,16> code_length_decoder_;
    huffman_code_length_decoder<boost::uint16_t> code_length_huffman_decoder_;
    huffman_code_length_decoder<boost::uint16_t> symbol_huffman_decoder_;
    huffman_code_length_decoder<boost::uint16_t> offset_size_huffman_decoder_;

    template<class InputBitStream>
    void decode_code_length_huffman_tree(InputBitStream& bs)
    {
        code_length_huffman_decoder_.clear();

        std::size_t size = bs.read_bits(5);
        if (size == 0)
        {
            code_length_decoder_.assign(bs.read_bits(5));
            return;
        }

        code_length_huffman_decoder_.reserve(size);
        for (std::size_t i = 0; i < size; ++i)
        {
            if (i == 3)
            {
                std::size_t count = bs.read_bits(2);
                code_length_huffman_decoder_.skip(count);
                i += count;

                if (i > size)
                    throw std::runtime_error("LZH invalid zero-run-length");
                if (i == size)
                    break;
            }
            code_length_huffman_decoder_.push_back(decode_code_length(bs));
        }
        code_length_huffman_decoder_.decode(code_length_decoder_);
    }

    template<class InputBitStream>
    void decode_symbol_huffman_tree_aux(InputBitStream& bs)
    {
        symbol_huffman_decoder_.clear();

        std::size_t size = bs.read_bits(9);
        if (size == 0)
        {
            symbol_decoder_.assign(bs.read_bits(9));
            return;
        }

        symbol_huffman_decoder_.reserve(size);
        for (std::size_t i = 0; i < size; )
        {
            boost::uint16_t n = code_length_decoder_.decode(bs);
            if (n == 0)
            {
                symbol_huffman_decoder_.skip(1);
                ++i;
            }
            else if (n == 1)
            {
                std::size_t count = bs.read_bits(4) + 3;
                symbol_huffman_decoder_.skip(count);
                i += count;
            }
            else if (n == 2)
            {
                std::size_t count = bs.read_bits(9) + 20;
                symbol_huffman_decoder_.skip(count);
                i += count;
            }
            else
            {
                symbol_huffman_decoder_.push_back(n-2);
                ++i;
            }
        }
        symbol_huffman_decoder_.decode(symbol_decoder_);
    }

    template<class InputBitStream>
    void decode_symbol_huffman_tree(InputBitStream& bs)
    {
        decode_code_length_huffman_tree(bs);
        decode_symbol_huffman_tree_aux(bs);
    }

    template<class InputBitStream>
    void decode_offset_size_huffman_tree(InputBitStream& bs)
    {
        offset_size_huffman_decoder_.clear();

        std::size_t size = bs.read_bits(offset_count_bits_);
        if (size == 0)
        {
            offset_decoder_.assign(bs.read_bits(offset_count_bits_));
            return;
        }

        offset_size_huffman_decoder_.reserve(size);
        for (std::size_t i = 0; i < size; ++i)
            offset_size_huffman_decoder_.push_back(decode_code_length(bs));
        offset_size_huffman_decoder_.decode(offset_decoder_);
    }
};

class lzhuf_output_impl
{
public:
    typedef char char_type;

    struct category
        : public boost::iostreams::output
        , public boost::iostreams::filter_tag
        , public boost::iostreams::multichar_tag
        , public boost::iostreams::flushable_tag
    {};

    typedef boost::uint16_t length_type;
    typedef boost::uint16_t offset_type;
    typedef literal_or_reference<offset_type,length_type> result_type;

    static const length_type min_match_length = 3;

    explicit lzhuf_output_impl(std::size_t window_bits)
    {
        if (window_bits <= 13)
            offset_count_bits_ = 4;
        else
            offset_count_bits_ = 5;
    }

    template<class Sink>
    bool flush(Sink& sink)
    {
        filter_.flush(sink);
        return true;
    }

    template<class Sink>
    std::streamsize write(Sink& sink, const char* s, std::streamsize n)
    {
        if (n <= 0)
            return 0;

        boost::uint16_t count = update_huffman_tree(s, n);
        write_header(sink, count);

        using boost::iostreams::array_source;
        typedef boost::iostreams::detail::
            direct_adapter<array_source> source_type;

        source_type src(array_source(s, s+n));

        hamigaki::iostreams::detail::
            modified_lzss_input<left_to_right,little,16,8> in;

        output_bit_stream<left_to_right,Sink> bs(filter_, sink);
        while (true)
        {
            const result_type& data = in.get(src);
            if (data.is_reference)
            {
                if (data.length == 0)
                    break;

                symbol_encoder_.encode(bs, 256+data.length-min_match_length);

                boost::uint8_t bits = bit_length(data.offset);
                offset_length_encoder_.encode(bs, bits);
                if (bits)
                    bs.write_bits(data.offset & ~(1<<bits), bits-1);
            }
            else
            {
                unsigned char uc = static_cast<unsigned char>(data.literal);
                symbol_encoder_.encode(bs, uc);
            }
        }
        return n;
    }

private:
    std::size_t offset_count_bits_;
    huffman_encoder<boost::uint16_t,16> symbol_encoder_;
    huffman_encoder<boost::uint16_t,16> offset_length_encoder_;
    huffman_encoder<boost::uint16_t,16> code_length_encoder_;
    huffman<boost::uint16_t> symbol_huffman_;
    huffman<boost::uint16_t> offset_length_huffman_;
    huffman<boost::uint16_t> code_length_huffman_;
    output_bit_filter<left_to_right> filter_;

    boost::uint8_t bit_length(boost::uint16_t n)
    {
        static const boost::uint8_t table[] =
        {
            0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4
        };

        if ((n & 0xFF00) == 0)
        {
            if ((n & 0xF0) == 0)
                return table[n];
            else
                return 4 + table[n >> 4];
        }
        else
        {
            if ((n & 0xF000) == 0)
                return 8 + table[n >> 8];
            else
                return 12 + table[n >> 12];
        }
    }

    void update_code_length_huffman_tree()
    {
        code_length_huffman_.clear();

        unsigned zero = 0;
        std::size_t size = symbol_encoder_.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            if (std::size_t bits = symbol_encoder_[i].bits)
            {
                if (zero)
                {
                    if (zero == 1)
                        code_length_huffman_.insert(0);
                    else if (zero == 2)
                    {
                        code_length_huffman_.insert(0);
                        code_length_huffman_.insert(0);
                    }
                    else if (zero < 19)
                        code_length_huffman_.insert(1);
                    else if (zero == 19)
                    {
                        code_length_huffman_.insert(0);
                        code_length_huffman_.insert(1);
                    }
                    else
                        code_length_huffman_.insert(2);
                }
                code_length_huffman_.insert(bits + 2);
                zero = 0;
            }
            else
            {
                if (++zero == 531)
                {
                    code_length_huffman_.insert(2);
                    zero = 0;
                }
            }
        }
        BOOST_ASSERT(zero == 0);

        code_length_huffman_.make_encoder(code_length_encoder_);
    }

    boost::uint16_t update_huffman_tree(const char* s, std::streamsize n)
    {
        using boost::iostreams::array_source;
        typedef boost::iostreams::detail::
            direct_adapter<array_source> source_type;

        source_type src(array_source(s, s+n));

        hamigaki::iostreams::detail::
            modified_lzss_input<left_to_right,little,16,8> in;

        symbol_huffman_.clear();
        offset_length_huffman_.clear();
        boost::uint16_t count = 0;
        while (true)
        {
            const result_type& data = in.get(src);
            if (data.is_reference)
            {
                if (data.length == 0)
                    break;

                symbol_huffman_.insert(256 + data.length - min_match_length);
                offset_length_huffman_.insert(bit_length(data.offset));
            }
            else
            {
                unsigned char uc = static_cast<unsigned char>(data.literal);
                symbol_huffman_.insert(uc);
            }
            ++count;
        }

        symbol_huffman_.make_encoder(symbol_encoder_);
        offset_length_huffman_.make_encoder(offset_length_encoder_);

        update_code_length_huffman_tree();

        return count;
    }

    template<class OutputBitStream>
    void write_zero_run_length(OutputBitStream& bs, unsigned n)
    {
        if (n == 1)
            code_length_encoder_.encode(bs, 0);
        else if (n == 2)
        {
            code_length_encoder_.encode(bs, 0);
            code_length_encoder_.encode(bs, 0);
        }
        else if (n < 19)
        {
            code_length_encoder_.encode(bs, 1);
            bs.write_bits(n-3, 4);
        }
        else if (n == 19)
        {
            code_length_encoder_.encode(bs, 0);
            code_length_encoder_.encode(bs, 1);
            bs.write_bits(0x0F, 4);
        }
        else
        {
            code_length_encoder_.encode(bs, 2);
            bs.write_bits(n-20, 9);
        }
    }

    template<class Sink>
    void write_header(Sink& sink, boost::uint16_t count)
    {
        output_bit_stream<left_to_right,Sink> bs(filter_, sink);
        bs.write_bits(count, 16);

        if (code_length_encoder_.empty())
        {
            bs.write_bits(0, 5);
            bs.write_bits(code_length_encoder_.default_value(), 5);
        }
        else
        {
            std::size_t size = code_length_encoder_.size();
            bs.write_bits(size, 5);

            std::size_t i = 0;
            for ( ; (i < 3) && (i < size); ++i)
                encode_code_length(bs, code_length_encoder_[i].bits);

            if (i < size)
            {
                unsigned z = 0;
                for ( ; (z < 3) && (i < size); ++z)
                {
                    if (code_length_encoder_[i].bits != 0)
                        break;
                    ++i;
                }
                bs.write_bits(z, 2);

                for ( ; i < size; ++i)
                    encode_code_length(bs, code_length_encoder_[i].bits);
            }
        }

        if (symbol_encoder_.empty())
        {
            bs.write_bits(0, 9);
            bs.write_bits(symbol_encoder_.default_value(), 9);
        }
        else
        {
            std::size_t size = symbol_encoder_.size();
            bs.write_bits(size, 9);

            unsigned zero = 0;
            for (std::size_t i = 0; i < size; ++i)
            {
                if (std::size_t bits = symbol_encoder_[i].bits)
                {
                    if (zero)
                        write_zero_run_length(bs, zero);
                    code_length_encoder_.encode(bs, 2+bits);
                    zero = 0;
                }
                else
                {
                    if (++zero == 531)
                    {
                        code_length_encoder_.encode(bs, 2);
                        bs.write_bits(0x1FF, 9);
                        zero = 0;
                    }
                }
            }
            BOOST_ASSERT(zero == 0);
        }

        if (offset_length_encoder_.empty())
        {
            bs.write_bits(0, offset_count_bits_);
            bs.write_bits(
                offset_length_encoder_.default_value(), offset_count_bits_);
        }
        else
        {
            std::size_t size = offset_length_encoder_.size();
            bs.write_bits(size, offset_count_bits_);
            for (std::size_t i = 0; i < size; ++i)
                encode_code_length(bs, offset_length_encoder_[i].bits);
        }
    }
};

class lzhuf_output
{
public:
    typedef boost::uint16_t length_type;
    typedef boost::uint16_t offset_type;

    static const length_type min_match_length = 3;
    static const length_type max_match_length = 256;
    static const std::size_t default_buffer_size = 16*1024;

    explicit lzhuf_output(
        std::size_t window_bits, std::size_t buffer_size=default_buffer_size)
        : impl_(window_bits), huffman_buffer_(buffer_size)
    {
    }

    template<class Sink>
    bool flush(Sink& sink)
    {
        boost::iostreams::composite<
            boost::reference_wrapper<lzhuf_output_impl>,
            boost::reference_wrapper<Sink>
        > impl(boost::ref(impl_), boost::ref(sink));
        huffman_buffer_.flush(impl);
        return impl.flush();
    }

    template<class Sink>
    bool flush(boost::reference_wrapper<Sink>& sink)
    {
        boost::iostreams::composite<
            boost::reference_wrapper<lzhuf_output_impl>,
            boost::reference_wrapper<Sink>
        > impl(boost::ref(impl_), sink);
        huffman_buffer_.flush(impl);
        return impl.flush();
    }

    template<class Sink>
    void put(Sink& sink, char literal)
    {
        boost::iostreams::composite<
            boost::reference_wrapper<lzhuf_output_impl>,
            boost::reference_wrapper<Sink>
        > impl(boost::ref(impl_), boost::ref(sink));
        huffman_buffer_.put(impl, literal);
    }

    template<class Sink>
    void put(boost::reference_wrapper<Sink>& sink, char literal)
    {
        boost::iostreams::composite<
            boost::reference_wrapper<lzhuf_output_impl>,
            boost::reference_wrapper<Sink>
        > impl(boost::ref(impl_), sink);
        huffman_buffer_.put(impl, literal);
    }

    template<class Sink>
    void put(Sink& sink, offset_type offset, length_type length)
    {
        boost::iostreams::composite<
            boost::reference_wrapper<lzhuf_output_impl>,
            boost::reference_wrapper<Sink>
        > impl(boost::ref(impl_), boost::ref(sink));
        huffman_buffer_.put(impl, offset, length);
    }

    template<class Sink>
    void put(
        boost::reference_wrapper<Sink>& sink,
        offset_type offset, length_type length)
    {
        boost::iostreams::composite<
            boost::reference_wrapper<lzhuf_output_impl>,
            boost::reference_wrapper<Sink>
        > impl(boost::ref(impl_), sink);
        huffman_buffer_.put(impl, offset, length);
    }

private:
    lzhuf_output_impl impl_;
    hamigaki::iostreams::detail::
        modified_lzss_output<left_to_right,little,16,8> huffman_buffer_;
};

} // namespace lha_detail

class lzhuf_decompressor
    : public sliding_window_decompress<lha_detail::lzhuf_input>
{
    typedef sliding_window_decompress<lha_detail::lzhuf_input> base_type;

public:
    explicit lzhuf_decompressor(std::size_t window_bits)
        : base_type(lha_detail::lzhuf_input(window_bits), window_bits)
    {
    }
};

class lzhuf_compressor
    : public sliding_window_compress<lha_detail::lzhuf_output>
{
    typedef sliding_window_compress<lha_detail::lzhuf_output> base_type;

public:
    explicit lzhuf_compressor(std::size_t window_bits)
        : base_type(lha_detail::lzhuf_output(window_bits), window_bits)
    {
    }

    lzhuf_compressor(std::size_t window_bits, std::size_t buffer_size)
        : base_type(lha_detail::lzhuf_output(window_bits, buffer_size)
        , window_bits)
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_LZHUF_HPP
