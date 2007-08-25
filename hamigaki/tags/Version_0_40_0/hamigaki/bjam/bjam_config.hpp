// bjam_config.hpp: configuration file for Hamigaki.Bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_BJAM_CONFIG_HPP
#define HAMIGAKI_BJAM_BJAM_CONFIG_HPP

#include <boost/config.hpp>

#if !defined(PHOENIX_LIMIT)
    #define PHOENIX_LIMIT 6
#endif

#if PHOENIX_LIMIT < 6
    #error "Hamigaki.Bjam requires PHOENIX_LIMIT >= 6"
#endif


#if !defined(HAMIGAKI_BJAM_SEPARATE_GRAMMAR_INSTANTIATION)
    #define HAMIGAKI_BJAM_SEPARATE_GRAMMAR_INSTANTIATION 1
#endif


#if defined(BOOST_HAS_DECLSPEC)
    #if defined(HAMIGAKI_ALL_DYN_LINK) || defined(HAMIGAKI_BJAM_DYN_LINK)
        #if defined(HAMIGAKI_BJAM_SOURCE)
            #define HAMIGAKI_BJAM_DECL __declspec(dllexport)
        #else
            #define HAMIGAKI_BJAM_DECL __declspec(dllimport)
        #endif
    #endif
#endif // defined(BOOST_HAS_DECLSPEC)

#if !defined(HAMIGAKI_BJAM_DECL)
    #define HAMIGAKI_BJAM_DECL
#endif

#if !defined(BOOST_ALL_NO_LIB) && \
    !defined(HAMIGAKI_BJAM_NO_LIB) && \
    !defined(HAMIGAKI_BJAM_SOURCE)

    #define BOOST_LIB_NAME hamigaki_bjam
    #if defined(BOOST_ALL_DYN_LINK) || defined(HAMIGAKI_BJAM_DYN_LINK)
        #define BOOST_DYN_LINK
    #endif
    #include <hamigaki/config/auto_link.hpp>
#endif

#endif // HAMIGAKI_BJAM_BJAM_CONFIG_HPP
