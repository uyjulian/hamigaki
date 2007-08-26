// relative_restrict.hpp: the relative access version of restriction

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

// Original Copyright
// ===========================================================================>
// (C) Copyright Jonathan Turkanis 2005.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.
// <===========================================================================

#ifndef HAMIGAKI_IOSTREAMS_RELATIVE_RESTRICT_HPP
#define HAMIGAKI_IOSTREAMS_RELATIVE_RESTRICT_HPP

#include <hamigaki/iostreams/detail/error.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/optimal_buffer_size.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/traits.hpp>

namespace hamigaki { namespace iostreams {

template<class Device>
class relative_restriction
{
public:
    typedef typename boost::iostreams::
        char_type_of<Device>::type char_type;

    struct category
        : boost::iostreams::mode_of<Device>::type
        , boost::iostreams::device_tag
        , boost::iostreams::optimally_buffered_tag
    {};

    relative_restriction(Device& dev,
        boost::iostreams::stream_offset off,
        boost::iostreams::stream_offset len=-1) : dev_ptr_(&dev)
    {
        beg_ = boost::iostreams::position_to_offset(
            boost::iostreams::seek(*dev_ptr_, off, BOOST_IOS::cur));
        pos_ = beg_;
        end_ = (len != -1) ? beg_ + len : -1;
    }

    std::streamsize optimal_buffer_size() const
    {
        return boost::iostreams::optimal_buffer_size(*dev_ptr_);
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        std::streamsize amt =
            end_ != -1
            ? (std::min)(n, static_cast<std::streamsize>(end_ - pos_))
            : n;
        if (amt == 0)
            return -1;

        std::streamsize result = boost::iostreams::read(*dev_ptr_, s, amt);
        if (result != -1)
            pos_ += result;
        return result;
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        if ((end_ != -1) && (pos_ + n >= end_))
            throw out_of_restriction("bad write");

        std::streamsize result = boost::iostreams::write(*dev_ptr_, s, n);
        pos_ += result;
        return result;
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        boost::iostreams::stream_offset next;
        if (way == BOOST_IOS::beg)
            next = beg_ + off;
        else if (way == BOOST_IOS::cur)
            next = pos_ + off;
        else if (end_ != -1)
            next = end_ + off;
        else
        {
            pos_ = boost::iostreams::position_to_offset(
                boost::iostreams::seek(*dev_ptr_, off, BOOST_IOS::end));

            if (pos_ < beg_)
                throw out_of_restriction("bad seek");

            return boost::iostreams::offset_to_position(pos_ - beg_);
        }

        if ((next < beg_) || ((end_ != -1) && (next > end_)))
            throw out_of_restriction("bad seek");

        pos_ = boost::iostreams::position_to_offset(
            boost::iostreams::seek(*dev_ptr_, next, BOOST_IOS::beg));
        return boost::iostreams::offset_to_position(pos_ - beg_);
    }

private:
    Device* dev_ptr_;
    boost::iostreams::stream_offset beg_;
    boost::iostreams::stream_offset pos_;
    boost::iostreams::stream_offset end_;
};

template<class Device>
inline relative_restriction<Device>
relative_restrict(Device& dev,
    boost::iostreams::stream_offset off,
    boost::iostreams::stream_offset len=-1)
{
    return relative_restriction<Device>(dev, off, len);
}

} } // End namespaces iostreams, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::relative_restriction, 1)

#endif // HAMIGAKI_IOSTREAMS_RELATIVE_RESTRICT_HPP
