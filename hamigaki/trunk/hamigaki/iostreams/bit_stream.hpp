//  bit_stream.hpp: bit stream

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BIT_STREAM_HPP
#define HAMIGAKI_IOSTREAMS_BIT_STREAM_HPP

#include <boost/iostreams/detail/error.hpp>
#include <boost/iostreams/get.hpp>
#include <boost/iostreams/put.hpp>

namespace hamigaki { namespace iostreams {

enum bit_flow
{
    left_to_right,
    right_to_left
};

template<bit_flow Flow, class Source>
class input_bit_stream;

template<class Source>
class input_bit_stream<left_to_right, Source>
{
public:
    explicit input_bit_stream(Source& src)
        : src_(src), count_(8)
    {
    }

    bool get_bit()
    {
        if (count_ == 8)
        {
            typedef std::char_traits<char> traits;

            traits::int_type c = boost::iostreams::get(src_);
            if (c == traits::eof())
                throw boost::iostreams::detail::bad_read();
            uc_ = static_cast<unsigned char>(traits::to_char_type(c));
            count_ = 0;
        }

        return (uc_ & (1 << (7-count_++))) != 0;
    }

    unsigned read_bits(std::size_t bit_count)
    {
        unsigned tmp = 0;
        while (bit_count--)
            tmp |= (static_cast<unsigned>(get_bit()) << bit_count);
        return tmp;
    }

private:
    Source& src_;
    unsigned char uc_;
    std::size_t count_;
};


template<bit_flow Flow, class Sink>
class output_bit_stream;

template<class Sink>
class output_bit_stream<left_to_right, Sink>
{
public:
    explicit output_bit_stream(Sink& sink)
        : sink_(sink), uc_(0), count_(0)
    {
    }

    ~output_bit_stream()
    {
        try
        {
            flush();
        }
        catch (...)
        {
        }
    }

    void flush()
    {
        if (count_ != 0)
        {
            boost::iostreams::put(sink_, static_cast<char>(uc_));
            count_ = 0;
        }
    }

    void put_bit(bool bit)
    {
        uc_ |= static_cast<unsigned>(bit) << (7-count_);

        if (++count_ == 8)
        {
            boost::iostreams::put(sink_, static_cast<char>(uc_));
            count_ = 0;
        }
    }

    void write_bits(unsigned bits, std::size_t bit_count)
    {
        while (bit_count--)
            write_bits(((bits >> bit_count) & 1) != 0);
    }

private:
    Sink& sink_;
    unsigned char uc_;
    std::size_t count_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_BIT_STREAM_HPP
