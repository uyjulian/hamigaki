// dont_close.hpp: a view with empty close()

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DONT_CLOSE_HPP
#define HAMIGAKI_IOSTREAMS_DONT_CLOSE_HPP

#include <hamigaki/iostreams/detail/device_adapter.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/flush.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>

namespace hamigaki { namespace iostreams {

template<class Device>
class dont_close_device
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

    explicit dont_close_device(const Device& dev)
        : base_type(dev)
    {
    }

    void close(BOOST_IOS::openmode which = BOOST_IOS::in | BOOST_IOS::out)
    {
        boost::iostreams::flush(this->component());
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return boost::iostreams::read(this->component(), s, n);
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        return boost::iostreams::write(this->component(), s, n);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return boost::iostreams::seek(
            this->component(), off, way);
    }
};

template<class Device>
inline dont_close_device<Device>
dont_close(const Device& dev)
{
    return dont_close_device<Device>(dev);
}

} } // End namespaces iostreams, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::dont_close_device, 1)

#endif // HAMIGAKI_IOSTREAMS_DONT_CLOSE_HPP
