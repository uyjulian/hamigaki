//  cyg_tls.hpp: the Cygwin TLS support functions

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#ifndef HAMIGAKI_COROUTINE_DETAIL_CYGWIN_CYG_TLS_HPP
#define HAMIGAKI_COROUTINE_DETAIL_CYGWIN_CYG_TLS_HPP

#include <cstring>

extern "C" {

unsigned long cygwin_internal(int, ...);

} // extern "C"

namespace hamigaki { namespace coroutine { namespace detail {

struct fiber_context
{
    char pad0[8];               // 00
    unsigned long stack_base;   // 08
    char pad1[0xCC];            // 0C
    unsigned long esp;          // D8
};

inline unsigned long cyg_tls_pad_size()
{
    // 33 = CW_CYGTLS_PADSIZE
    return ::cygwin_internal(33);
}

#define HAMIGAKI_COROUTINE_DETAIL_ALLOCA_CYG_TLS() \
    alloca(hamigaki::coroutine::detail::cyg_tls_pad_size())

inline void copy_cyg_tls_first(void* to, void* from)
{
    fiber_context* callee = reinterpret_cast<fiber_context*>(to);
    fiber_context* caller = reinterpret_cast<fiber_context*>(from);
    unsigned long pad_size = cyg_tls_pad_size();
    std::memcpy(
        reinterpret_cast<void*>(callee->stack_base - pad_size),
        reinterpret_cast<void*>(caller->stack_base - pad_size),
        pad_size
    );
}

inline void copy_cyg_tls(void* to, void* from)
{
    fiber_context* callee = reinterpret_cast<fiber_context*>(to);
    fiber_context* caller = reinterpret_cast<fiber_context*>(from);
    unsigned long pad_size = cyg_tls_pad_size();
    if (callee->stack_base - callee->esp >= pad_size)
    {
        std::memcpy(
            reinterpret_cast<void*>(callee->stack_base - pad_size),
            reinterpret_cast<void*>(caller->stack_base - pad_size),
            pad_size
        );
    }
}

} } } // End namespaces detail, coroutine, hamigaki.

#endif // HAMIGAKI_COROUTINE_DETAIL_CYGWIN_CYG_TLS_HPP
