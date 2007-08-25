// lzss.hpp: LZSS compression/decompression

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_FILTER_LZSS_HPP
#define HAMIGAKI_IOSTREAMS_FILTER_LZSS_HPP

#include <hamigaki/iostreams/filter/sliding_window.hpp>
#include <hamigaki/iostreams/bit_stream.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/integer.hpp>

namespace hamigaki { namespace iostreams {

namespace detail
{

template<bit_flow Flow, std::size_t OffsetBits, std::size_t LengthBits>
class lzss_input
{
public:
    typedef typename boost::uint_t<OffsetBits>::least offset_type;
    typedef typename boost::uint_t<LengthBits+1>::least length_type;
    typedef literal_or_reference<offset_type,length_type> result_type;

    static const length_type min_match_length = 3;

    template<class Source>
    result_type get(Source& src)
    {
        input_bit_stream<Flow, Source> bs(filter_, src);
        if (bs.get_bit())
        {
            offset_type offset =
                static_cast<offset_type>(bs.read_bits(OffsetBits));

            length_type length =
                static_cast<length_type>(bs.read_bits(LengthBits)) +
                min_match_length;

            return result_type(offset, length);
        }
        else
        {
            char literal =
                static_cast<char>(static_cast<unsigned char>(bs.read_bits(8)));

            return result_type(literal);
        }
    }

private:
    input_bit_filter<Flow> filter_;
};

template<bit_flow Flow, std::size_t OffsetBits, std::size_t LengthBits>
class lzss_output
{
public:
    typedef typename boost::uint_t<OffsetBits>::least offset_type;
    typedef typename boost::uint_t<LengthBits+1>::least length_type;

    static const length_type min_match_length = 3;

    static const length_type max_match_length =
        (1u<<LengthBits) + min_match_length - 1;

    template<class Sink>
    bool flush(Sink& sink)
    {
        output_bit_stream<Flow, Sink> bs(filter_, sink);
        bs.flush();
        return true;
    }

    template<class Sink>
    void put(Sink& sink, char literal)
    {
        output_bit_stream<Flow, Sink> bs(filter_, sink);
        bs.put_bit(0);
        bs.write_bits(static_cast<unsigned char>(literal), 8);
    }

    template<class Sink>
    void put(Sink& sink, offset_type offset, length_type length)
    {
        output_bit_stream<Flow, Sink> bs(filter_, sink);
        bs.put_bit(1);
        bs.write_bits(offset, OffsetBits);
        bs.write_bits(length-min_match_length, LengthBits);
    }

private:
    output_bit_filter<Flow> filter_;
};

} // namespace detail

template<bit_flow Flow, std::size_t OffsetBits, std::size_t LengthBits>
class lzss_decompressor
    : public sliding_window_decompress<
        detail::lzss_input<Flow,OffsetBits,LengthBits>
    >
{
    typedef detail::lzss_input<Flow,OffsetBits,LengthBits> input_type;
    typedef sliding_window_decompress<input_type> base_type;

public:
    explicit lzss_decompressor(std::size_t window_bits)
        : base_type(input_type(), window_bits)
    {
    }
};

template<bit_flow Flow, std::size_t OffsetBits, std::size_t LengthBits>
class lzss_compressor
    : public sliding_window_compress<
        detail::lzss_output<Flow,OffsetBits,LengthBits>
    >
{
    typedef detail::lzss_output<Flow,OffsetBits,LengthBits> output_type;
    typedef sliding_window_compress<output_type> base_type;

public:
    explicit lzss_compressor(std::size_t window_bits)
        : base_type(output_type(), window_bits)
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_LZSS_HPP
