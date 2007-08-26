// bit_stream.hpp: bit stream

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BIT_STREAM_HPP
#define HAMIGAKI_IOSTREAMS_BIT_STREAM_HPP

#include <hamigaki/iostreams/bit_filter.hpp>

namespace hamigaki { namespace iostreams {

template<bit_flow Flow, class Source>
class input_bit_stream
{
public:
    input_bit_stream(input_bit_filter<Flow>& filter, Source& src)
        : filter_(filter), src_(src)
    {
    }

    bool get_bit()
    {
        return filter_.get_bit(src_);
    }

    unsigned read_bits(std::size_t bit_count)
    {
        return filter_.read_bits(src_, bit_count);
    }

private:
    input_bit_filter<Flow>& filter_;
    Source& src_;
};

template<bit_flow Flow, class Sink>
class output_bit_stream
{
public:
    output_bit_stream(output_bit_filter<Flow>& filter, Sink& sink)
        : filter_(filter), sink_(sink)
    {
    }

    void flush()
    {
        return filter_.flush(sink_);
    }

    void put_bit(bool bit)
    {
        return filter_.put_bit(sink_, bit);
    }

    void write_bits(unsigned bits, std::size_t bit_count)
    {
        return filter_.write_bits(sink_, bits, bit_count);
    }

private:
    output_bit_filter<Flow>& filter_;
    Sink& sink_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_BIT_STREAM_HPP
