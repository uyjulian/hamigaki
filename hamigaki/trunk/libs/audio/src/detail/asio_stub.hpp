// asio_stub.hpp: ASIO API stub functions

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_HPP
#define HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_HPP

#if defined(__GNUC__) && defined(__i386__)
    #include "./asio_stub_gcc_x86.hpp"
#elif defined(__BORLANDC__) && defined(_M_IX86)
    #include "./asio_stub_bcc_x86.hpp"
#else
    #include "./asio_stub_default.hpp"
#endif

#endif // HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_HPP
