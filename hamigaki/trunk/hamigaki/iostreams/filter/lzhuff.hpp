//  lzhuff.hpp: LZHUFF compression/decompression

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_FILTER_LZHUFF_HPP
#define HAMIGAKI_IOSTREAMS_FILTER_LZHUFF_HPP

#include <hamigaki/iostreams/filter/sliding_window.hpp>
#include <hamigaki/iostreams/utility/huffman.hpp>
#include <hamigaki/iostreams/bit_filter.hpp>
#include <boost/iostreams/detail/ios.hpp>

namespace hamigaki { namespace iostreams {

namespace lha_detail
{

template<class InputBitStream>
boost::uint16_t decode_code_length(InputBitStream& bs)
{
    boost::uint16_t n = bs.read_bits(3);
    if (n == 7)
    {
        while (bs.get_bit())
            ++n;
    }
    return n;
}

template<class InputBitStream>
void decode_code_length_huffman_tree(
    InputBitStream& bs,
    huffman_decoder<boost::uint16_t,16>& tree)
{
    huffman_code_length_decoder<boost::uint16_t> decoder;

    std::size_t size = bs.read_bits(5);
    if (size == 0)
    {
        tree.assign(bs.read_bits(5));
        return;
    }

    decoder.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        if (i == 3)
        {
            std::size_t count = bs.read_bits(2);
            decoder.skip(count);
            i += count;

            if (i > size)
                throw std::runtime_error("LZH invalid zero-run-length");
            if (i == size)
                break;
        }
        decoder.push_back(decode_code_length(bs));
    }
    decoder.decode(tree);
}

template<class InputBitStream>
void decode_symbol_huffman_tree(
    InputBitStream& bs,
    huffman_decoder<boost::uint16_t,16>& length_table,
    huffman_decoder<boost::uint16_t,16>& tree)
{
    huffman_code_length_decoder<boost::uint16_t> decoder;

    std::size_t size = bs.read_bits(9);
    if (size == 0)
    {
        tree.assign(bs.read_bits(9));
        return;
    }

    decoder.reserve(size);
    for (std::size_t i = 0; i < size; )
    {
        boost::uint16_t n = length_table.decode(bs);
        if (n == 0)
        {
            decoder.skip(1);
            ++i;
        }
        else if (n == 1)
        {
            std::size_t count = bs.read_bits(4) + 3;
            decoder.skip(count);
            i += count;
        }
        else if (n == 2)
        {
            std::size_t count = bs.read_bits(9) + 20;
            decoder.skip(count);
            i += count;
        }
        else
        {
            decoder.push_back(n-2);
            ++i;
        }
    }
    decoder.decode(tree);
}

template<class InputBitStream>
void decode_symbol_huffman_tree(
    InputBitStream& bs,
    huffman_decoder<boost::uint16_t,16>& tree)
{
    huffman_decoder<boost::uint16_t,16> decoder;
    decode_code_length_huffman_tree(bs, decoder);
    decode_symbol_huffman_tree(bs, decoder, tree);
}

template<class InputBitStream>
void decode_offset_size_huffman_tree(
    InputBitStream& bs,
    huffman_decoder<boost::uint16_t,16>& tree)
{
    huffman_code_length_decoder<boost::uint16_t> decoder;

    std::size_t size = bs.read_bits(4);
    if (size == 0)
    {
        tree.assign(bs.read_bits(4));
        return;
    }

    decoder.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
        decoder.push_back(decode_code_length(bs));
    decoder.decode(tree);
}

class lzhuff_input
{
public:
    typedef boost::uint16_t length_type;
    typedef boost::uint16_t offset_type;
    typedef literal_or_reference<offset_type,length_type> result_type;

    static const length_type min_match_length = 3;

    explicit lzhuff_input(std::size_t size) : size_(size), pos_(0), count_(0)
    {
    }

    template<class Source>
    result_type get(Source& src)
    {
        if (pos_ == size_)
            return result_type(0, 0);

        input_bit_stream_wrapper<left_to_right, Source> bs(filter_, src);
        if (count_ == 0)
        {
            count_ = static_cast<boost::uint16_t>(bs.read_bits(16));
            decode_symbol_huffman_tree(bs, symbol_decoder_);
            decode_offset_size_huffman_tree(bs, offset_decoder_);
        }

        --count_;
        boost::uint16_t symbol = symbol_decoder_.decode(bs);
        if (symbol < 256)
        {
            char literal =
                static_cast<char>(static_cast<unsigned char>(symbol));
            ++pos_;
            return result_type(literal);
        }
        else
        {
            boost::uint16_t length = symbol - 256 + min_match_length;
            boost::uint16_t size = offset_decoder_.decode(bs);

            boost::uint16_t offset = size;
            if (size > 1)
                offset = (1 << (size-1)) | bs.read_bits(size-1);

            pos_ += length;
            if (pos_ > size_)
                throw BOOST_IOSTREAMS_FAILURE("LZH bad image size");
            return result_type(offset, length);
        }
    }

private:
    std::size_t size_;
    std::size_t pos_;
    std::size_t count_;
    input_bit_filter<left_to_right> filter_;
    huffman_decoder<boost::uint16_t,16> symbol_decoder_;
    huffman_decoder<boost::uint16_t,16> offset_decoder_;
};

} // namespace lha_detail

class lzhuff_decompressor
    : public sliding_window_decompress<lha_detail::lzhuff_input>
{
    typedef sliding_window_decompress<lha_detail::lzhuff_input> base_type;

public:
    lzhuff_decompressor(std::size_t window_bits, std::size_t size)
        : base_type(lha_detail::lzhuff_input(size), window_bits)
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_LZHUFF_HPP
