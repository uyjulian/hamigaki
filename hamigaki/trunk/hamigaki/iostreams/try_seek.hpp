// try_seek.hpp: seek() if possible

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_TRY_SEEK_HPP
#define HAMIGAKI_IOSTREAMS_TRY_SEEK_HPP

#include <boost/config.hpp>
#include <hamigaki/iostreams/positioning.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/seek.hpp>

namespace hamigaki { namespace iostreams {

namespace detail
{

template<typename T> 
struct try_seek_impl;

template<>
struct try_seek_impl<boost::iostreams::detail::random_access>
{
    template<typename T>
    static std::streampos
    seek(T& t, stream_offset off,
        BOOST_IOS::seekdir way, BOOST_IOS::openmode which)
    {
        return boost::iostreams::seek(t, off, way, which);
    }
};

template<>
struct try_seek_impl<boost::iostreams::any_tag>
{
    template<typename T>
    static std::streampos
    seek(T&, stream_offset, BOOST_IOS::seekdir, BOOST_IOS::openmode)
    {
        throw BOOST_IOSTREAMS_FAILURE("cannot seek");
        BOOST_UNREACHABLE_RETURN(std::streampos(std::streamoff(-1)))
    }
};

} // namespace detail

template<typename T>
inline std::streampos
try_seek(T& t, stream_offset off, BOOST_IOS::seekdir way,
    BOOST_IOS::openmode which = BOOST_IOS::in | BOOST_IOS::out)
{
    typedef typename boost::iostreams::detail::dispatch<
        T,
        boost::iostreams::detail::random_access,
        boost::iostreams::any_tag
    >::type tag;

    return detail::try_seek_impl<tag>::seek(t, off, way, which);
}

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_TRY_SEEK_HPP
