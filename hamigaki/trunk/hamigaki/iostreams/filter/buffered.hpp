// buffered_filter.hpp: buffered filter

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_FILTER_BUFFERED_HPP
#define HAMIGAKI_IOSTREAMS_FILTER_BUFFERED_HPP

#include <boost/iostreams/detail/adapter/non_blocking_adapter.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/assert.hpp>
#include <boost/shared_array.hpp>
#include <cstring>

namespace hamigaki { namespace iostreams {

class buffered_input_filter
{
public:
    typedef char char_type;

    struct category
        : public boost::iostreams::input
        , public boost::iostreams::filter_tag
    {};

    explicit buffered_input_filter(std::size_t buffer_size=4096)
        : buffer_(new char[buffer_size]), buffer_size_(buffer_size)
        , size_(0), index_(0)
    {
    }

    template<class Source>
    std::char_traits<char>::int_type get(Source& src)
    {
        if (index_ == size_)
        {
            while (true)
            {
                std::streamsize amt =
                    boost::iostreams::read(src, buffer_.get(), buffer_size_);
                if (amt == -1)
                    return std::char_traits<char>::eof();
                else if (amt)
                {
                    size_ = amt;
                    index_ = 0;
                    break;
                }
            }
        }

        return std::char_traits<char>::to_int_type(buffer_[index_++]);
    }

private:
    boost::shared_array<char> buffer_;
    std::size_t buffer_size_;
    std::size_t size_;
    std::size_t index_;
};


class buffered_output_filter
{
public:
    typedef char char_type;

    struct category
        : public boost::iostreams::output
        , public boost::iostreams::filter_tag
        , public boost::iostreams::flushable_tag
    {};

    explicit buffered_output_filter(std::size_t buffer_size=4096)
        : buffer_(new char[buffer_size]), buffer_size_(buffer_size), index_(0)
    {
    }

    template<class Sink>
    bool flush(Sink& sink)
    {
        if (index_ != 0)
        {
            std::streamsize size = static_cast<std::streamsize>(index_);

            boost::iostreams::non_blocking_adapter<Sink> nb(sink);
            std::streamsize amt =
                boost::iostreams::write(nb, buffer_.get(), size);
            index_ = 0;
            return amt == size;
        }
        return true;
    }

    template<class Sink>
    bool put(Sink& sink, char c)
    {
        buffer_[index_++] = c;
        if (index_ == buffer_size_)
            return this->flush(sink);
        return true;
    }

    std::size_t buffer_space() const
    {
        return buffer_size_ - index_;
    }

    template<class Sink>
    void write_buffer(Sink& sink, const char* s, std::streamsize n)
    {
        BOOST_ASSERT(index_+n < buffer_size_);

        std::memcpy(&buffer_[index_], s, n);
        index_ += n;
    }

private:
    boost::shared_array<char> buffer_;
    std::size_t buffer_size_;
    std::size_t index_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_FILTER_BUFFERED_HPP
