// skip.hpp: refinement of boost::iostreams::skip()

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

// Original Copyright
// ===========================================================================>
// (C) Copyright Jonathan Turkanis 2003.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.
// <===========================================================================

#ifndef HAMIGAKI_IOSTREAMS_SKIP_HPP
#define HAMIGAKI_IOSTREAMS_SKIP_HPP

#include <hamigaki/integer/auto_min.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/is_convertible.hpp>

namespace hamigaki { namespace iostreams {

namespace detail
{

template<typename Device>
void skip_impl(
    Device& dev, boost::iostreams::stream_offset off, boost::mpl::true_)
{
    boost::iostreams::seek(dev, off, BOOST_IOS::cur);
}

template<typename Device>
void skip_impl(
    Device& dev, boost::iostreams::stream_offset off, boost::mpl::false_)
{
    char buf[256];

    while (off > 0)
    {
        std::streamsize size = static_cast<std::streamsize>(sizeof(buf));
        size = auto_min(size, off);

        std::streamsize amt = boost::iostreams::read(dev, buf, size);
        if (amt == -1)
            throw BOOST_IOSTREAMS_FAILURE("bad skip offset");
        off -= amt;
    }
}

template<typename Filter, typename Device>
void skip_impl(
    Filter& flt, Device& dev,
    boost::iostreams::stream_offset off, boost::mpl::true_)
{
    boost::iostreams::seek(flt, dev, off, BOOST_IOS::cur);
}

template<typename Filter, typename Device>
void skip_impl(
    Filter& flt, Device& dev,
    boost::iostreams::stream_offset off, boost::mpl::false_)
{
    char buf[256];

    while (off > 0)
    {
        std::streamsize size = static_cast<std::streamsize>(sizeof(buf));
        size = auto_min(size, off);

        std::streamsize amt = boost::iostreams::read(flt, dev, buf, size);
        if (amt == -1)
            throw BOOST_IOSTREAMS_FAILURE("bad skip offset");
        off -= amt;
    }
}


} // namespace detail

template<typename Device>
void skip(Device& dev, boost::iostreams::stream_offset off)
{ 
    typedef typename boost::iostreams::mode_of<Device>::type mode;

    typedef boost::is_convertible<
        mode,
        boost::iostreams::input_seekable
    > can_seek;

    detail::skip_impl(dev, off, can_seek());
}

template<typename Filter, typename Device>
void skip(Filter& flt, Device& dev, boost::iostreams::stream_offset off)
{ 
    typedef typename boost::iostreams::mode_of<Filter>::type filter_mode;
    typedef typename boost::iostreams::mode_of<Device>::type device_mode;

    typedef boost::mpl::and_<
        boost::is_convertible<filter_mode, boost::iostreams::input_seekable>,
        boost::is_convertible<device_mode, boost::iostreams::input_seekable>
    > can_seek;

    detail::skip_impl(flt, dev, off, can_seek());
}

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_SKIP_HPP
