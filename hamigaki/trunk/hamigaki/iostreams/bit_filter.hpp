// bit_filter.hpp: bit stream filter

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BIT_FILTER_HPP
#define HAMIGAKI_IOSTREAMS_BIT_FILTER_HPP

#include <boost/iostreams/detail/error.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/shared_array.hpp>

namespace hamigaki { namespace iostreams {

enum bit_flow
{
    left_to_right,
    right_to_left
};

template<bit_flow Flow>
struct bit_stream_traits;

template<>
struct bit_stream_traits<left_to_right>
{
    static unsigned char mask(std::size_t bit)
    {
        static const unsigned char m[] =
        {
            0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
        };
        return m[bit];
    }
};

template<>
struct bit_stream_traits<right_to_left>
{
    static unsigned char mask(std::size_t bit)
    {
        static const unsigned char m[] =
        {
            0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
        };
        return m[bit];
    }
};

template<bit_flow Flow>
class input_bit_filter
{
public:
    typedef bit_stream_traits<Flow> traits_type;

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

        bool result = (buffer_[index_] & traits_type::mask(bit_)) != 0;
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


template<bit_flow Flow>
class output_bit_filter
{
public:
    typedef bit_stream_traits<Flow> traits_type;

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
        if (bit)
            buffer_[index_] |= traits_type::mask(bit_);

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

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_BIT_FILTER_HPP
