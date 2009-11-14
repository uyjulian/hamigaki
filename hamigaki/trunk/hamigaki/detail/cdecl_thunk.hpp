// cdecl_thunk.hpp: an instance thunk for __cdecl functions

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_CDECL_THUNK_HPP
#define HAMIGAKI_DETAIL_CDECL_THUNK_HPP

#if defined(_M_IX86) || defined(__i386__)
    #include <hamigaki/detail/i386/cdecl_thunk.hpp>
#elif defined(_M_AMD64)
    #include <hamigaki/detail/x64/cdecl_thunk.hpp>
#else
    #error "Sorry, unsupported architecture"
#endif

#endif // HAMIGAKI_DETAIL_CDECL_THUNK_HPP
