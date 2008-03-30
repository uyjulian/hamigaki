// modified_lzss.hpp: modified LZSS compression/decompression

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_FILTER_MODIFIED_LZSS_HPP
#define HAMIGAKI_IOSTREAMS_FILTER_MODIFIED_LZSS_HPP

#include <hamigaki/iostreams/filter/buffered.hpp>
#include <hamigaki/iostreams/filter/sliding_window.hpp>
#include <hamigaki/iostreams/bit_filter.hpp>
#include <hamigaki/binary/endian.hpp>
#include <boost/iostreams/detail/error.hpp>
#include <boost/iostreams/detail/ios.hpp>

namespace hamigaki { namespace iostreams {

namespace detail
{

template<
    bit_flow Flow, endianness E,
    std::size_t OffsetBits, std::size_t LengthBits>
class modified_lzss_input
{
    typedef bit_stream_traits<Flow> traits_type;

public:
    typedef typename boost::uint_t<OffsetBits>::least offset_type;
    typedef typename boost::uint_t<LengthBits+1>::least length_type;
    typedef literal_or_reference<offset_type,length_type> result_type;

    static const length_type min_match_length = 3;

    explicit modified_lzss_input(std::size_t buffer_size=4096)
        : filter_(buffer_size), bit_(8)
    {
    }

    template<class Source>
    result_type get(Source& src)
    {
        typedef std::char_traits<char> traits;

        if (bit_ == 8)
        {
            traits::int_type c = filter_.get(src);
            if (traits::eq_int_type(c, traits::eof()))
                return result_type(0, 0);

            flags_ = traits::to_char_type(c);
            bit_ = 0;
        }

        if ((flags_ & traits_type::mask(bit_++)) != 0)
        {
            length_type length = this->read_uint<Source,LengthBits>(src);
            length += min_match_length;

            offset_type offset = this->read_uint<Source,OffsetBits>(src);

            return result_type(offset, length);
        }
        else
        {
            traits::int_type c = filter_.get(src);
            if (traits::eq_int_type(c, traits::eof()))
            {
                bit_ = 8;
                return result_type(0, 0);
            }
            return result_type(traits::to_char_type(c));
        }
    }

private:
    buffered_input_filter filter_;
    std::size_t bit_;
    unsigned char flags_;

    template<class Source>
    char next(Source& src)
    {
        typedef std::char_traits<char> traits;
        traits::int_type c = filter_.get(src);
        if (traits::eq_int_type(c, traits::eof()))
            throw boost::iostreams::detail::bad_read();
        return traits::to_char_type(c);
    }

    template<class Source, std::size_t Bits>
    typename boost::uint_t<Bits>::least read_uint(Source& src)
    {
        typedef typename boost::uint_t<Bits>::least uint_type;
        char buf[sizeof(uint_type)];
        for (std::size_t i = 0; i < Bits/8; ++i)
            buf[i] = next(src);
        return hamigaki::decode_uint<E,Bits/8>(buf);
    }
};

template<
    bit_flow Flow, endianness E,
    std::size_t OffsetBits, std::size_t LengthBits>
class modified_lzss_output
{
    typedef bit_stream_traits<Flow> traits_type;

public:
    typedef typename boost::uint_t<OffsetBits>::least offset_type;
    typedef typename boost::uint_t<LengthBits+1>::least length_type;

    static const length_type min_match_length = 3;

    static const length_type max_match_length =
        (1u<<LengthBits) + min_match_length - 1;

    explicit modified_lzss_output(std::size_t buffer_size=4096)
        : filter_(buffer_size), bit_(0), index_(1)
    {
        buf_[0] = '\0';
    }

    template<class Sink>
    bool flush(Sink& sink)
    {
        if (bit_ != 0)
            flush_buffer(sink);
        filter_.flush(sink);
        return true;
    }

    template<class Sink>
    void put(Sink& sink, char literal)
    {
        buf_[index_++] = literal;

        if (++bit_ == 8)
            flush_buffer(sink);
    }

    template<class Sink>
    void put(Sink& sink, offset_type offset, length_type length)
    {
        buf_[0] = static_cast<unsigned char>(buf_[0]) | traits_type::mask(bit_);

        hamigaki::encode_uint<E,LengthBits/8>(
            &buf_[index_], length-min_match_length);
        index_ += LengthBits/8;

        hamigaki::encode_uint<E,OffsetBits/8>(&buf_[index_], offset);
        index_ += OffsetBits/8;

        if (++bit_ == 8)
            flush_buffer(sink);
    }

private:
    buffered_output_filter filter_;
    char buf_[1 + (LengthBits/8 + OffsetBits/8)*8];
    std::size_t bit_;
    std::streamsize index_;

    template<class Sink>
    void flush_buffer(Sink& sink)
    {
        filter_.write_buffer(sink, buf_, index_);
        if (filter_.buffer_space() < sizeof(buf_))
            filter_.flush(sink);
        buf_[0] = '\0';
        bit_ = 0;
        index_ = 1;
    }
};

} // namespace detail

template<
    bit_flow Flow, endianness E,
    std::size_t OffsetBits, std::size_t LengthBits>
class modified_lzss_decompressor
    : public sliding_window_decompress<
        detail::modified_lzss_input<Flow,E,OffsetBits,LengthBits>
    >
{
    typedef detail::modified_lzss_input<
        Flow,E,OffsetBits,LengthBits> input_type;
    typedef sliding_window_decompress<input_type> base_type;

public:
    explicit modified_lzss_decompressor(std::size_t window_bits)
        : base_type(input_type(), window_bits)
    {
    }

    modified_lzss_decompressor(std::size_t buffer_size, std::size_t window_bits)
        : base_type(input_type(buffer_size), window_bits)
    {
    }
};

template<
    bit_flow Flow, endianness E,
    std::size_t OffsetBits, std::size_t LengthBits>
class modified_lzss_compressor
    : public sliding_window_compress<
        detail::modified_lzss_output<Flow,E,OffsetBits,LengthBits>
    >
{
    typedef detail::modified_lzss_output<
        Flow,E,OffsetBits,LengthBits> output_type;
    typedef sliding_window_compress<output_type> base_type;

public:
    explicit modified_lzss_compressor(std::size_t window_bits)
        : base_type(output_type(), window_bits)
    {
    }

    modified_lzss_compressor(std::size_t buffer_size, std::size_t window_bits)
        : base_type(output_type(buffer_size), window_bits)
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_MODIFIED_LZSS_HPP
