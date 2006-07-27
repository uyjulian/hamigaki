//  lazy_restrict.hpp: boost:iostreams::restriction with lazy seek()

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Original Copyright
// ===========================================================================>
// (C) Copyright Jonathan Turkanis 2005.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

// See http://www.boost.org/libs/iostreams for documentation.
// <===========================================================================

#ifndef HAMIGAKI_IOSTREAMS_LAZY_SEEK_HPP
#define HAMIGAKI_IOSTREAMS_LAZY_SEEK_HPP

#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/adapter/basic_adapter.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>

namespace hamigaki { namespace iostreams {

template<class Device>
class lazy_restriction
    : public boost::iostreams::detail::basic_adapter<Device>
{
    typedef boost::iostreams::detail::basic_adapter<Device> base_type;

public:
    typedef typename boost::iostreams::
        char_type_of<Device>::type char_type;

    struct category :
        boost::iostreams::mode_of<Device>::type,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        boost::iostreams::flushable_tag,
        boost::iostreams::localizable_tag,
        boost::iostreams::optimally_buffered_tag {};

    lazy_restriction(const Device& dev,
        boost::iostreams::stream_offset off,
        boost::iostreams::stream_offset len=-1)
        : base_type(dev), beg_(off), pos_(-1)
        , end_(len != -1 ? off + len : -1)
    {
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        if (pos_ == -1)
        {
            pos_ = boost::iostreams::seek(
                this->component(), beg_, BOOST_IOS::beg);
        }

        std::streamsize amt =
            end_ != -1
            ? (std::min)(n, static_cast<std::streamsize>(end_ - pos_))
            : n;
        if (amt == 0)
            return -1;

        std::streamsize result =
            boost::iostreams::read(this->component(), s, amt);
        if (result != -1)
            pos_ += result;
        return result;
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        if (pos_ == -1)
        {
            pos_ = boost::iostreams::seek(
                this->component(), beg_, BOOST_IOS::beg);
        }

        if ((end_ != -1) && (pos_ + n >= end_))
            throw BOOST_IOSTREAMS_FAILURE("bad write");

        std::streamsize result =
            boost::iostreams::write(this->component(), s, n);
        if (result != -1)
            pos_ += result;
        return result;
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        if (pos_ == -1)
            pos_ = beg_;

        boost::iostreams::stream_offset next;
        if (way == BOOST_IOS::beg)
            next = beg_ + off;
        else if (way == BOOST_IOS::cur)
            next = pos_ + off;
        else if (end_ != -1)
            next = end_ + off;
        else
        {
            pos_ = boost::iostreams::seek(
                this->component(), off, BOOST_IOS::end);

            if (pos_ < beg_)
                throw BOOST_IOSTREAMS_FAILURE("bad seek");

            return boost::iostreams::offset_to_position(pos_ - beg_);
        }

        if ((next < beg_) || ((end_ != -1) && (next >= end_)))
            throw BOOST_IOSTREAMS_FAILURE("bad seek");

        pos_ = boost::iostreams::seek(
            this->component(), next, BOOST_IOS::beg);
        return boost::iostreams::offset_to_position(pos_ - beg_);
    }

private:
    boost::iostreams::stream_offset beg_;
    boost::iostreams::stream_offset pos_;
    boost::iostreams::stream_offset end_;
};

template<class Device>
inline lazy_restriction<Device>
lazy_restrict(const Device& dev,
    boost::iostreams::stream_offset off,
    boost::iostreams::stream_offset len=-1)
{
    return lazy_restriction<Device>(dev, off, len);
}

} } // End namespaces iostreams, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::lazy_restriction, 1)

#endif // HAMIGAKI_IOSTREAMS_LAZY_SEEK_HPP
