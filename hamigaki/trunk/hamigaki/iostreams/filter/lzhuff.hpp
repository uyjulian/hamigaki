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

class lzhuff_input
{
public:
    typedef boost::uint16_t length_type;
    typedef boost::uint16_t offset_type;
    typedef literal_or_reference<offset_type,length_type> result_type;

    static const length_type min_match_length = 3;

    explicit lzhuff_input(std::size_t window_bits) : count_(0)
    {
        if (window_bits <= 13)
            offset_count_bits_ = 4;
        else
            offset_count_bits_ = 5;
    }

    template<class Source>
    result_type get(Source& src)
    {
        input_bit_stream_wrapper<left_to_right, Source> bs(filter_, src);
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

} // namespace lha_detail

class lzhuff_decompressor
    : public sliding_window_decompress<lha_detail::lzhuff_input>
{
    typedef sliding_window_decompress<lha_detail::lzhuff_input> base_type;

public:
    explicit lzhuff_decompressor(std::size_t window_bits)
        : base_type(lha_detail::lzhuff_input(window_bits), window_bits)
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_LZHUFF_HPP
