// config.hpp: configuration file for Hamigaki.Process

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_DETAIL_CONFIG_HPP
#define HAMIGAKI_PROCESS_DETAIL_CONFIG_HPP

#include <boost/config.hpp>

#if defined(BOOST_HAS_DECLSPEC)
    #if defined(HAMIGAKI_ALL_DYN_LINK) || defined(HAMIGAKI_PROCESS_DYN_LINK)
        #if defined(HAMIGAKI_PROCESS_SOURCE)
            #define HAMIGAKI_PROCESS_DECL __declspec(dllexport)
        #else
            #define HAMIGAKI_PROCESS_DECL __declspec(dllimport)
        #endif
    #endif
#endif // defined(BOOST_HAS_DECLSPEC)

#if !defined(HAMIGAKI_PROCESS_DECL)
    #define HAMIGAKI_PROCESS_DECL
#endif

#endif // HAMIGAKI_PROCESS_DETAIL_CONFIG_HPP
