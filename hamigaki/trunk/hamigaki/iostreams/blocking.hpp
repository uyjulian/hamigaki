// blocking.hpp: the specialization of non_blocking_adapter for Blocking

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BLOCKING_HPP
#define HAMIGAKI_IOSTREAMS_BLOCKING_HPP

#include <boost/iostreams/detail/adapter/non_blocking_adapter.hpp>
#include <boost/iostreams/detail/error.hpp>
#include <boost/iostreams/detail/template_params.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/write.hpp>
#include <new>
#include <string>

namespace hamigaki { namespace iostreams {

template<class Source>
struct blocking_reader
{
    typedef typename boost::iostreams::char_type_of<Source>::type char_type;

    static char_type* read(Source& src, char_type* s, std::streamsize n)
    {
        boost::iostreams::non_blocking_adapter<Source> nb(src);
        if (boost::iostreams::read(nb, s, n) != n)
            return 0;
        return s;
    }
};

template<class Sink>
struct blocking_writer
{
    typedef typename boost::iostreams::char_type_of<Sink>::type char_type;

    static bool write(Sink& sink, const char_type* s, std::streamsize n)
    {
        boost::iostreams::non_blocking_adapter<Sink> nb(sink);
        return (boost::iostreams::write(nb, s, n) == n);
    }
};

template<class Source>
inline char* blocking_read(Source& src, char* s, std::streamsize n)
{
    if (!blocking_reader<Source>::read(src, s, n))
        throw boost::iostreams::detail::bad_read();
    return s;
}

template<class Source>
inline char* blocking_read(
    Source& src, char* s, std::streamsize n, const std::nothrow_t&)
{
    return blocking_reader<Source>::read(src, s, n);
}

template<class Source>
inline char blocking_get(Source& src)
{
    char c;
    iostreams::blocking_read(src, &c, 1);
    return c;
}

template<class Source>
inline int blocking_get(Source& src, const std::nothrow_t&)
{
    char c;
    if (!blocking_reader<Source>::read(src, &c, 1))
        return std::char_traits<char>::eof();
    return std::char_traits<char>::to_int_type(c);
}

template<class Source, std::size_t Size>
inline char* blocking_read(Source& src, char (&s)[Size])
{
    return iostreams::blocking_read(src, s, static_cast<std::streamsize>(Size));
}

template<class Source, std::size_t Size>
inline char* blocking_read(Source& src, char (&s)[Size], const std::nothrow_t&)
{
    return blocking_reader<Source>::
        read(src, s, static_cast<std::streamsize>(Size));
}

template<class Sink>
inline void blocking_write(Sink& sink, const char* s, std::streamsize n)
{
    if (!blocking_writer<Sink>::write(sink, s, n))
        throw boost::iostreams::detail::bad_write();
}

template<class Sink>
inline bool blocking_write(
    Sink& sink, const char* s, std::streamsize n, const std::nothrow_t&)
{
    return blocking_writer<Sink>::write(sink, s, n);
}

template<class Sink>
inline void blocking_put(Sink& sink, char c)
{
    iostreams::blocking_write(sink, &c, 1);
}

template<class Sink>
inline bool blocking_put(Sink& sink, char c, const std::nothrow_t&)
{
    return blocking_writer<Sink>::write(sink, &c, 1);
}

template<class Sink, std::size_t Size>
inline void blocking_write(Sink& sink, const char (&s)[Size])
{
    iostreams::blocking_write(sink, s, static_cast<std::streamsize>(Size));
}

template<class Sink, std::size_t Size>
inline bool blocking_write(
    Sink& sink, const char (&s)[Size], const std::nothrow_t&)
{
    return blocking_writer<Sink>::
        write(sink, s, static_cast<std::streamsize>(Size));
}

template<class Sink>
inline void blocking_write(Sink& sink, const std::string& s)
{
    iostreams::blocking_write(
        sink, s.c_str(), static_cast<std::streamsize>(s.size()));
}

template<class Sink>
inline bool blocking_write(
    Sink& sink, const std::string& s, const std::nothrow_t&)
{
    return blocking_writer<Sink>::
        write(sink, s.c_str(), static_cast<std::streamsize>(s.size()));
}

} } // End namespaces iostreams, hamigaki.

#define HAMIGAKI_IOSTREAMS_BLOCKING(device, arity) \
namespace boost { namespace iostreams { \
    template< \
        BOOST_PP_ENUM_PARAMS(arity, typename T) \
    > \
    class non_blocking_adapter< \
        device BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T) \
    > \
    { \
        typedef device BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T) device_type; \
    public: \
        typedef BOOST_PP_EXPR_IF(arity, typename) \
            char_type_of<device_type>::type char_type; \
        struct category \
            : boost::iostreams::mode_of<device_type>::type \
            , boost::iostreams::device_tag {}; \
        explicit non_blocking_adapter(device_type& dev) : device_(dev) {} \
        std::streamsize read(char_type* s, std::streamsize n) \
        { return boost::iostreams::read(device_, s, n); } \
        std::streamsize write(const char_type* s, std::streamsize n) \
        { return boost::iostreams::write(device_, s, n); } \
        std::streampos seek( \
            boost::iostreams::stream_offset off, BOOST_IOS::seekdir way, \
            BOOST_IOS::openmode which = BOOST_IOS::in | BOOST_IOS::out) \
        { return boost::iostreams::seek(device_, off, way, which); } \
    public: \
        device_type& device_; \
    }; \
}} \
/**/

#define HAMIGAKI_IOSTREAMS_BLOCKING_SOURCE(source, arity) \
namespace hamigaki { namespace iostreams { \
    template< \
        BOOST_PP_ENUM_PARAMS(arity, typename T) \
    > \
    struct blocking_reader< \
        source BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T) \
    > \
    { \
        typedef source BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T) source_type; \
        typedef BOOST_PP_EXPR_IF(arity, typename) \
            boost::iostreams::char_type_of<source_type>::type char_type; \
        static char_type* read( \
            source_type& src, char_type* s, std::streamsize n) \
        { \
            if (boost::iostreams::read(src, s, n) != n) \
                return 0; \
            return s; \
        } \
    }; \
}} \
/**/

#define HAMIGAKI_IOSTREAMS_BLOCKING_SINK(sink, arity) \
namespace hamigaki { namespace iostreams { \
    template< \
        BOOST_PP_ENUM_PARAMS(arity, typename T) \
    > \
    struct blocking_writer< \
        sink BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T) \
    > \
    { \
        typedef sink BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T) sink_type; \
        typedef BOOST_PP_EXPR_IF(arity, typename) \
            boost::iostreams::char_type_of<sink_type>::type char_type; \
        static bool write( \
            sink_type& src, const char_type* s, std::streamsize n) \
        { \
            return boost::iostreams::write(src, s, n) == n; \
        } \
    }; \
}} \
/**/

HAMIGAKI_IOSTREAMS_BLOCKING_SOURCE(boost::iostreams::non_blocking_adapter, 1)
HAMIGAKI_IOSTREAMS_BLOCKING_SINK(boost::iostreams::non_blocking_adapter, 1)

#endif // HAMIGAKI_IOSTREAMS_BLOCKING_HPP
