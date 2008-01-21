// config.hpp: configuration file for Hamigaki.Input

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_INPUT_DETAIL_CONFIG_HPP
#define HAMIGAKI_INPUT_DETAIL_CONFIG_HPP

#include <boost/config.hpp>

#if defined(BOOST_HAS_DECLSPEC)
    #if defined(BOOST_ALL_DYN_LINK) || defined(HAMIGAKI_INPUT_DYN_LINK)
        #if defined(HAMIGAKI_INPUT_SOURCE)
            #define HAMIGAKI_INPUT_DECL __declspec(dllexport)
        #else
            #define HAMIGAKI_INPUT_DECL __declspec(dllimport)
        #endif
    #endif
#endif // defined(BOOST_HAS_DECLSPEC)

#if !defined(HAMIGAKI_INPUT_DECL)
    #define HAMIGAKI_INPUT_DECL
#endif

#endif // HAMIGAKI_INPUT_DETAIL_CONFIG_HPP
