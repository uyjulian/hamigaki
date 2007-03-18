//  config.hpp: configuration file for Hamigaki.Filesystem

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_FILESYSTEM_DETAIL_CONFIG_HPP
#define HAMIGAKI_FILESYSTEM_DETAIL_CONFIG_HPP

#include <boost/config.hpp>

#if defined(BOOST_HAS_DECLSPEC)
    #if defined(HAMIGAKI_ALL_DYN_LINK) || defined(HAMIGAKI_FILESYSTEM_DYN_LINK)
        #if defined(HAMIGAKI_FILESYSTEM_SOURCE)
            #define HAMIGAKI_FILESYSTEM_DECL __declspec(dllexport)
        #else
            #define HAMIGAKI_FILESYSTEM_DECL __declspec(dllimport)
        #endif
    #endif
#endif // defined(BOOST_HAS_DECLSPEC)

#if !defined(HAMIGAKI_FILESYSTEM_DECL)
    #define HAMIGAKI_FILESYSTEM_DECL
#endif

#endif // HAMIGAKI_FILESYSTEM_DETAIL_CONFIG_HPP
