// bjam_config.hpp: configuration file for Hamigaki.Bjam

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_BJAM_CONFIG_HPP
#define HAMIGAKI_BJAM2_BJAM_CONFIG_HPP

#include <boost/config.hpp>

#if !defined(HAMIGAKI_BJAM2_SEPARATE_GRAMMAR_INSTANTIATION)
    #define HAMIGAKI_BJAM2_SEPARATE_GRAMMAR_INSTANTIATION 1
#endif


#if defined(BOOST_HAS_DECLSPEC)
    #if defined(HAMIGAKI_ALL_DYN_LINK) || defined(HAMIGAKI_BJAM2_DYN_LINK)
        #if defined(HAMIGAKI_BJAM2_SOURCE)
            #define HAMIGAKI_BJAM2_DECL __declspec(dllexport)
        #else
            #define HAMIGAKI_BJAM2_DECL __declspec(dllimport)
        #endif
    #endif
#endif // defined(BOOST_HAS_DECLSPEC)

#if !defined(HAMIGAKI_BJAM2_DECL)
    #define HAMIGAKI_BJAM2_DECL
#endif

#if !defined(BOOST_ALL_NO_LIB) && \
    !defined(HAMIGAKI_BJAM2_NO_LIB) && \
    !defined(HAMIGAKI_BJAM2_SOURCE)

    #define BOOST_LIB_NAME hamigaki_bjam
    #if defined(BOOST_ALL_DYN_LINK) || defined(HAMIGAKI_BJAM2_DYN_LINK)
        #define BOOST_DYN_LINK
    #endif
    #include <hamigaki/config/auto_link.hpp>
#endif

#endif // HAMIGAKI_BJAM2_BJAM_CONFIG_HPP
