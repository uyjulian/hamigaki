//  bit_filter.hpp: bit stream filter

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BIT_FILTER_HPP
#define HAMIGAKI_IOSTREAMS_BIT_FILTER_HPP

#include <hamigaki/iostreams/bit_stream.hpp>

namespace hamigaki { namespace iostreams {

template<bit_flow Flow>
class input_bit_filter;

template<>
class input_bit_filter<left_to_right>
{
public:
    input_bit_filter() : count_(8)
    {
    }

    template<class Source>
    bool get_bit(Source& src)
    {
        if (count_ == 8)
        {
            typedef std::char_traits<char> traits;

            traits::int_type c = boost::iostreams::get(src);
            if (traits::eq_int_type(c, traits::eof()))
                throw boost::iostreams::detail::bad_read();
            uc_ = static_cast<unsigned char>(traits::to_char_type(c));
            count_ = 0;
        }

        return (uc_ & (1 << (7-count_++))) != 0;
    }

    template<class Source>
    unsigned read_bits(Source& src, std::size_t bit_count)
    {
        unsigned tmp = 0;
        while (bit_count--)
            tmp |= (static_cast<unsigned>(this->get_bit(src)) << bit_count);
        return tmp;
    }

private:
    unsigned char uc_;
    std::size_t count_;
};

template<bit_flow Flow, class Source>
class input_bit_stream_wrapper
{
public:
    input_bit_stream_wrapper(input_bit_filter<Flow>& filter, Source& src)
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


template<bit_flow Flow>
class output_bit_filter;

template<>
class output_bit_filter<left_to_right>
{
public:
    output_bit_filter() : uc_(0), count_(0)
    {
    }

    template<class Sink>
    void flush(Sink& sink)
    {
        if (count_ != 0)
        {
            boost::iostreams::put(sink, static_cast<char>(uc_));
            count_ = 0;
        }
    }

    template<class Sink>
    void put_bit(Sink& sink, bool bit)
    {
        uc_ |= static_cast<unsigned>(bit) << (7-count_);

        if (++count_ == 8)
        {
            boost::iostreams::put(sink, static_cast<char>(uc_));
            count_ = 0;
        }
    }

    template<class Sink>
    void write_bits(Sink& sink, unsigned bits, std::size_t bit_count)
    {
        while (bit_count--)
            this->write_bits(sink, ((bits >> bit_count) & 1) != 0);
    }

private:
    unsigned char uc_;
    std::size_t count_;
};

template<bit_flow Flow, class Sink>
class output_bit_stream_wrapper
{
public:
    output_bit_stream_wrapper(output_bit_filter<Flow>& filter, Sink& sink)
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

#endif // HAMIGAKI_IOSTREAMS_BIT_FILTER_HPP
