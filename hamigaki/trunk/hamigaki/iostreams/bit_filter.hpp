//  bit_filter.hpp: bit stream filter

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BIT_FILTER_HPP
#define HAMIGAKI_IOSTREAMS_BIT_FILTER_HPP

#include <hamigaki/iostreams/bit_stream.hpp>
#include <boost/shared_array.hpp>

namespace hamigaki { namespace iostreams {

template<bit_flow Flow>
class input_bit_filter;

template<>
class input_bit_filter<left_to_right>
{
public:
    static const std::size_t buffer_size = 4096;

    input_bit_filter()
        : buffer_(new char[buffer_size]), size_(0), index_(0), bit_(0)
    {
    }

    template<class Source>
    bool get_bit(Source& src)
    {
        if (index_ == size_)
        {
            while (true)
            {
                std::streamsize amt =
                    boost::iostreams::read(src, buffer_.get(), buffer_size);
                if (amt == -1)
                    throw boost::iostreams::detail::bad_read();
                else if (amt)
                {
                    size_ = amt;
                    index_ = 0;
                    bit_ = 0;
                    break;
                }
            }
        }

        static const unsigned char mask[] =
        {
            0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
        };
        bool result = (buffer_[index_] & mask[bit_]) != 0;
        if (++bit_ == 8)
        {
            bit_ = 0;
            ++index_;
        }
        return result;
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
    boost::shared_array<char> buffer_;
    std::size_t size_;
    std::size_t index_;
    std::size_t bit_;
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
    static const std::size_t buffer_size = 4096;

    output_bit_filter()
        : buffer_(new char[buffer_size]), index_(0), bit_(0)
    {
        std::memset(buffer_.get(), 0, buffer_size);
    }

    template<class Sink>
    void flush(Sink& sink)
    {
        if (bit_ != 0)
        {
            bit_ = 0;
            ++index_;
        }

        if (index_ != 0)
        {
            boost::iostreams::write(sink, buffer_.get(), index_);
            std::memset(buffer_.get(), 0, index_);
            index_ = 0;
        }
    }

    template<class Sink>
    void put_bit(Sink& sink, bool bit)
    {
        static const unsigned char mask[] =
        {
            0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
        };

        if (bit)
            buffer_[index_] |= mask[bit_];

        if (++bit_ == 8)
        {
            bit_ = 0;
            if (++index_ == buffer_size)
            {
                boost::iostreams::write(sink, buffer_.get(), buffer_size);
                std::memset(buffer_.get(), 0, buffer_size);
                index_ = 0;
            }
        }
    }

    template<class Sink>
    void write_bits(Sink& sink, unsigned bits, std::size_t bit_count)
    {
        while (bit_count--)
            this->put_bit(sink, ((bits >> bit_count) & 1) != 0);
    }

private:
    boost::shared_array<char> buffer_;
    std::size_t index_;
    std::size_t bit_;
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
