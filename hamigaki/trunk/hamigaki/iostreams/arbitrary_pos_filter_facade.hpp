//  arbitrary_pos_filter_facade.hpp: repositional stream filter facade

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_IOSTREAMS_ARBITRARY_POS_FILTER_FACADE_HPP
#define HAMIGAKI_IOSTREAMS_ARBITRARY_POS_FILTER_FACADE_HPP

#include <hamigaki/iostreams/core_access.hpp>

namespace hamigaki { namespace iostreams {

template<class Derived, class CharT, std::streamsize MaxBlockSize>
class arbitrary_pos_filter_facade
{
private:
    typedef CharT char_type;

    Derived& derived()
    {
      return *static_cast<Derived*>(this);
    }

protected:
    typedef arbitrary_pos_filter_facade<
        Derived,CharT,MaxBlockSize> arbitrary_pos_filter_facade_;

    void block_size(std::streamsize n)
    {
        block_size_ = n;
    }

public:
    arbitrary_pos_filter_facade() : block_size_(MaxBlockSize), count_(0)
    {
    }

    explicit arbitrary_pos_filter_facade(std::streamsize block_size)
        : block_size_(block_size), count_(0)
    {
        BOOST_ASSERT(block_size_ <= MaxBlockSize);
    }

    template<class Source>
    std::streamsize read(const Source& src, char_type* s, std::streamsize n)
    {
        std::streamsize total = 0;

        if (count_ != 0)
        {
            std::streamsize amt = (std::min)(n, count_);
            char_type* start = buffer_ + (block_size_ - count_);
            s = std::copy(start, start+amt, s);
            n -= amt;
            count_ -= amt;
            total += amt;
        }

        if (n >= block_size_)
        {
            BOOST_ASSERT(count_ == 0);

            std::streamsize request = n/block_size_;
            std::streamsize res =
                core_access::read_blocks(derived(), src, s, request);

            if (res != -1)
            {
                std::streamsize amt = res*block_size_;
                s += amt;
                n -= amt;
                total += amt;
            }

            if (res < request)
                return total != 0 ? total : -1;
        }

        if (n != 0)
        {
            BOOST_ASSERT(n < block_size_);
            BOOST_ASSERT(count_ == 0);

            std::streamsize res =
                core_access::read_blocks(derived(), src, buffer_, 1);

            if (res == 1)
            {
                s = std::copy(buffer_, buffer_+n, s);
                count_ = block_size_ - n;
                total += n;
            }
        }

        return total != 0 ? total : -1;
    }

    template<class Sink>
    std::streamsize write(Sink& sink, const char_type* s, std::streamsize n)
    {
        std::streamsize total = 0;

        if (count_ != 0)
        {
            std::streamsize amt = (std::min)(n, block_size_-count_);
            std::copy(s, s+amt, buffer_+count_);
            s += amt;
            n -= amt;
            count_ += amt;
            total += amt;

            if (count_ == block_size_)
            {
                std::streamsize res =
                    core_access::write_blocks(derived(), sink, buffer_, 1);
                if (res)
                count_ = 0;
            }
        }

        if (n >= block_size_)
        {
            BOOST_ASSERT(count_ == 0);

            std::streamsize request = n/block_size_;
            std::streamsize res =
                core_access::write_blocks(derived(), sink, s, request);

            if (res != -1)
            {
                std::streamsize amt = res*block_size_;
                s += amt;
                n -= amt;
                total += amt;
            }

            if (res < request)
                return total != 0 ? total : -1;
        }

        if (n != 0)
        {
            BOOST_ASSERT(n < block_size_);
            BOOST_ASSERT(count_ == 0);

            std::copy(s, s+n, buffer_);
            count_ = n;
            total += n;
        }

        return total != 0 ? total : -1;
    }

    template<class Sink>
    void close(Sink& sink)
    {
        BOOST_ASSERT(count_ < block_size_);
        core_access::close_with_flush(derived(), sink, buffer_, count_);
    }

private:
    char_type buffer_[MaxBlockSize];
    std::streamsize block_size_;
    std::streamsize count_;
};


} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_ARBITRARY_POS_FILTER_FACADE_HPP
