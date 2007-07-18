// sum8.hpp: 8bit checksum

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#ifndef HAMIGAKI_CHECKSUM_SUM8_HPP
#define HAMIGAKI_CHECKSUM_SUM8_HPP

#include <cstddef>
#include <numeric>

namespace hamigaki { namespace checksum {

class sum8
{
public:
    typedef unsigned char value_type;

    sum8() : sum_(0)
    {
    }

    void reset(value_type new_sum = 0)
    {
        sum_ = new_sum;
    }

    void process_byte(unsigned char byte)
    {
        sum_ += byte;
    }

    void process_block(const void* bytes_begin, const void* bytes_end)
    {
        sum_ +=
            std::accumulate(
                static_cast<const unsigned char*>(bytes_begin),
                static_cast<const unsigned char*>(bytes_end),
                0u
            );
    }

    void process_bytes(const void* buffer, std::size_t byte_count)
    {
        sum_ +=
            std::accumulate(
                static_cast<const unsigned char*>(buffer),
                static_cast<const unsigned char*>(buffer) + byte_count,
                0u
            );
    }

    value_type checksum() const
    {
        return static_cast<unsigned char>(sum_);
    }

    void operator()(unsigned char byte)
    {
        sum_ += byte;
    }

    value_type operator()() const
    {
        return static_cast<unsigned char>(sum_);
    }

private:
    unsigned sum_;
};

} } // End namespaces checksum, hamigaki.

#endif // HAMIGAKI_CHECKSUM_SUM8_HPP
