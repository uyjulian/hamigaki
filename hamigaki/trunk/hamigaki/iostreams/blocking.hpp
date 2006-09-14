//  blocking.hpp: the specialization of non_blocking_adapter for Blocking

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BLOCKING_HPP
#define HAMIGAKI_IOSTREAMS_BLOCKING_HPP

#include <boost/iostreams/detail/adapter/non_blocking_adapter.hpp>
#include <boost/iostreams/detail/template_params.hpp>

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

#endif // HAMIGAKI_IOSTREAMS_BLOCKING_HPP
