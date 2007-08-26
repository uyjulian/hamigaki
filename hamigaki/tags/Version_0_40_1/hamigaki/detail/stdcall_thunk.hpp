// stdcall_thunk.hpp: an instance thunk for __stdcall functions

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_STDCALL_THUNK_HPP
#define HAMIGAKI_DETAIL_STDCALL_THUNK_HPP

#if defined(_M_IX86) || defined(__i386__)
    #include <hamigaki/detail/i386/stdcall_thunk.hpp>
#else
    #error "Sorry, unsupported architecture"
#endif

#endif // HAMIGAKI_DETAIL_STDCALL_THUNK_HPP
