// catable.hpp: operator +/* for Souces

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_CATABLE_HPP
#define HAMIGAKI_IOSTREAMS_CATABLE_HPP

#include <hamigaki/iostreams/concatenate.hpp>
#include <hamigaki/iostreams/repeat.hpp>
#include <boost/iostreams/detail/template_params.hpp>

#define HAMIGAKI_IOSTREAMS_CATABLE(source, arity) \
    namespace hamigaki{namespace iostreams{namespace cat_operators{ \
        template< BOOST_PP_ENUM_PARAMS(arity, typename T) \
                  BOOST_PP_COMMA_IF(arity) typename Component> \
        ::hamigaki::iostreams::concatenation< \
            source BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T), \
            Component \
        > operator+( const source BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T)& s, \
                     const Component& c ) \
        { \
            return ::hamigaki::iostreams::concatenation< \
                       source BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T), \
                       Component> \
                       (s, c); \
        } \
        template< BOOST_PP_ENUM_PARAMS(arity, typename T) \
                  BOOST_PP_COMMA_IF(arity) typename Component > \
        ::hamigaki::iostreams::repetition< \
            source BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T) \
        > operator*( const source BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T)& s, \
                     Component count ) \
        { \
            return ::hamigaki::iostreams::repetition< \
                       source BOOST_IOSTREAMS_TEMPLATE_ARGS(arity, T) > \
                       (s, count); \
        } \
    }}} \
    /**/

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::concatenation,2)
HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::repetition, 1)

#endif // HAMIGAKI_IOSTREAMS_CATABLE_HPP
