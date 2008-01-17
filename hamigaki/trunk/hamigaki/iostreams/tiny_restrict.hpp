// tiny_restrict.hpp: boost:iostreams::restriction without seek()

// Copyright Takeshi Mouri 2006-2008.
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

#ifndef HAMIGAKI_IOSTREAMS_TINY_RESTRICT_HPP
#define HAMIGAKI_IOSTREAMS_TINY_RESTRICT_HPP

#include <hamigaki/iostreams/detail/device_adapter.hpp>
#include <hamigaki/iostreams/detail/error.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>

namespace hamigaki { namespace iostreams {

template<class Device>
class tiny_restriction
    : public HAMIGAKI_IOSTREAMS_DEVICE_ADAPTER<Device>
{
    typedef HAMIGAKI_IOSTREAMS_DEVICE_ADAPTER<Device> base_type;

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

    tiny_restriction(const Device& dev,
        boost::iostreams::stream_offset len)
        : base_type(dev), pos_(0)
        , end_(len != -1 ? len : -1)
    {
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
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
        if ((end_ != -1) && (pos_ + n >= end_))
            throw out_of_restriction("bad write");

        std::streamsize result =
            boost::iostreams::write(this->component(), s, n);
        pos_ += result;
        return result;
    }

private:
    boost::iostreams::stream_offset pos_;
    boost::iostreams::stream_offset end_;
};

template<class Device>
inline tiny_restriction<Device>
tiny_restrict(const Device& dev,
    boost::iostreams::stream_offset len)
{
    return tiny_restriction<Device>(dev, len);
}

} } // End namespaces iostreams, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::tiny_restriction, 1)

#endif // HAMIGAKI_IOSTREAMS_TINY_RESTRICT_HPP
