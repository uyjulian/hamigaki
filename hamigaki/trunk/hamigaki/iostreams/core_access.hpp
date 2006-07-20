//  core_access.hpp: CRTP access helper

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_IOSTREAMS_CORE_ACCESS_HPP
#define HAMIGAKI_IOSTREAMS_CORE_ACCESS_HPP

#include <boost/config.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/traits.hpp>

namespace hamigaki { namespace iostreams {

template<class Derived, class CharT, std::streamsize MaxBlockSize>
class arbitrary_pos_device_facade;

template<class Derived, class CharT, std::streamsize MaxBlockSize>
class arbitrary_pos_filter_facade;

class core_access
{
#if defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
public:
#else
    template<class Derived, class CharT, std::streamsize MaxBlockSize>
    friend class arbitrary_pos_device_facade;

    template<class Derived, class CharT, std::streamsize MaxBlockSize>
    friend class arbitrary_pos_filter_facade;
#endif

    template<class RepositionalSource, class CharT>
    static std::streamsize read_blocks(
        RepositionalSource& src, CharT* s, std::streamsize n)
    {
        return src.read_blocks(s, n);
    }

    template<class RepositionalInputFilter, class Source>
    static std::streamsize read_blocks(
        RepositionalInputFilter& filter, Source& src,
        typename boost::iostreams::char_type_of<Source>::type* s,
        std::streamsize n)
    {
        return filter.read_blocks(src, s, n);
    }

    template<class RepositionalSink, class CharT>
    static std::streamsize write_blocks(
        RepositionalSink& sink, const CharT* s, std::streamsize n)
    {
        return sink.write_blocks(s, n);
    }

    template<class RepositionalOutputFilter, class Sink>
    static std::streamsize write_blocks(
        RepositionalOutputFilter& filter, Sink& sink,
        const typename boost::iostreams::char_type_of<Sink>::type* s,
        std::streamsize n)
    {
        return filter.write_blocks(sink, s, n);
    }

    template<class RepositionalSink, class CharT>
    static void close_with_flush(
        RepositionalSink& sink, const CharT* s, std::streamsize n)
    {
        return sink.close_with_flush(s, n);
    }

    template<class RepositionalOutputFilter, class Sink>
    static void close_with_flush(
        RepositionalOutputFilter& filter, Sink& sink,
        const typename boost::iostreams::char_type_of<Sink>::type* s,
        std::streamsize n)
    {
        return filter.close_with_flush(sink, s, n);
    }

    template<class RepositionalDevice>
    static std::streampos seek_blocks(
        RepositionalDevice& dev,
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return dev.seek_blocks(off, way);
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_CORE_ACCESS_HPP
