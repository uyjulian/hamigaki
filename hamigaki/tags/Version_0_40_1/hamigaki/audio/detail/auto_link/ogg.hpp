// ogg.hpp: auto-linking for libogg

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_AUTO_LINK_OGG_HPP
#define HAMIGAKI_AUDIO_DETAIL_AUTO_LINK_OGG_HPP

#if defined(HAMIGAKI_OGG_BINARY)
    #define BOOST_LIB_NAME HAMIGAKI_OGG_BINARY
    #define BOOST_AUTO_LINK_NOMANGLE
    #if defined(BOOST_ALL_DYN_LINK) || defined(HAMIGAKI_AUDIO_DYN_LINK)
        #define BOOST_DYN_LINK
    #endif
    #include <boost/config/auto_link.hpp>
#else
    #if !defined(BOOST_ALL_NO_LIB) && \
        !defined(HAMIGAKI_AUDIO_NO_LIB) && \
        !defined(HAMIGAKI_AUDIO_SOURCE)

        #define BOOST_LIB_NAME hamigaki_ogg
        #if defined(BOOST_ALL_DYN_LINK) || defined(HAMIGAKI_AUDIO_DYN_LINK)
            #define BOOST_DYN_LINK
        #endif
        #include <hamigaki/config/auto_link.hpp>
    #endif
#endif

#endif // HAMIGAKI_AUDIO_DETAIL_AUTO_LINK_OGG_HPP
