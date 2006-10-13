//  seek.hpp: the extensions of boost::iostreams::seek()

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_SEEK_HPP
#define HAMIGAKI_IOSTREAMS_SEEK_HPP

#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/seek.hpp>

namespace hamigaki { namespace iostreams {

typedef boost::iostreams::stream_offset stream_offset;

inline std::streampos to_position(stream_offset n)
{
    return boost::iostreams::offset_to_position(n);
}

inline stream_offset to_offset(std::streampos pos)
{
    return boost::iostreams::position_to_offset(pos);
}

template<typename T>
inline std::streampos
seek(T& t, std::streampos pos,
    BOOST_IOS::openmode which = BOOST_IOS::in | BOOST_IOS::out)
{ 
    return boost::iostreams::seek(t,
        boost::iostreams::position_to_offset(pos), BOOST_IOS::beg, which);
}

template<typename T, typename Device>
inline std::streampos
seek(T& t, Device& dev, std::streampos pos,
    BOOST_IOS::openmode which = BOOST_IOS::in | BOOST_IOS::out)
{ 
    return boost::iostreams::seek(t, dev,
        boost::iostreams::position_to_offset(pos), BOOST_IOS::beg, which);
}


template<typename T>
inline std::streampos
tell(T& t, BOOST_IOS::openmode which = BOOST_IOS::in | BOOST_IOS::out)
{
    return boost::iostreams::seek(t, 0, BOOST_IOS::cur, which);
}

template<typename T, typename Device>
inline std::streampos
tell(T& t, Device& dev,
    BOOST_IOS::openmode which = BOOST_IOS::in | BOOST_IOS::out)
{
    return boost::iostreams::seek(t, dev, 0, BOOST_IOS::cur, which);
}


template<typename T>
inline stream_offset
tell_offset(T& t, BOOST_IOS::openmode which = BOOST_IOS::in | BOOST_IOS::out)
{
    return boost::iostreams::position_to_offset(iostreams::tell(t, which));
}

template<typename T, typename Device>
inline stream_offset
tell_offset(T& t, Device& dev,
    BOOST_IOS::openmode which = BOOST_IOS::in | BOOST_IOS::out)
{
    return boost::iostreams::position_to_offset(iostreams::tell(t, dev, which));
}

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_SEEK_HPP
