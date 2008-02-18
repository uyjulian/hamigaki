// hello.hpp: Hello library

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HELLO_HPP
#define HELLO_HPP

#include <boost/config.hpp>

#if defined(BOOST_HAS_DECLSPEC)
    #if defined(BOOST_ALL_DYN_LINK) || defined(HELLO_DYN_LINK)
        #ifdef HELLO_SOURCE
            #define HELLO_DECL __declspec(dllexport)
        #else
            #define HELLO_DECL __declspec(dllimport)
        #endif
    #endif
#endif

#if !defined(HELLO_DECL)
    #define HELLO_DECL
#endif

HELLO_DECL void hello();

#endif // #ifndef HELLO_HPP
