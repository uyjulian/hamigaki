// auto_link.hpp: auto-linking for Hamigaki.Input

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/input for library home page.

#ifndef HAMIGAKI_INPUT_DETAIL_AUTO_LINK_HPP
#define HAMIGAKI_INPUT_DETAIL_AUTO_LINK_HPP

#if !defined(BOOST_ALL_NO_LIB) && \
    !defined(HAMIGAKI_INPUT_NO_LIB) && \
    !defined(HAMIGAKI_INPUT_SOURCE)

    #define BOOST_LIB_NAME hamigaki_input
    #if defined(BOOST_ALL_DYN_LINK) || defined(HAMIGAKI_INPUT_DYN_LINK)
        #define BOOST_DYN_LINK
    #endif
    #include <hamigaki/config/auto_link.hpp>
#endif

#endif // HAMIGAKI_INPUT_DETAIL_AUTO_LINK_HPP
