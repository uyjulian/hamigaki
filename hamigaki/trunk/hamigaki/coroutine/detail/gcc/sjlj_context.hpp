// sjlj_context.hpp: the context of gcc EH using setjmp/longjmp

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#ifndef HAMIGAKI_COROUTINE_DETAIL_GCC_SJLJ_CONTEXT_HPP
#define HAMIGAKI_COROUTINE_DETAIL_GCC_SJLJ_CONTEXT_HPP

extern "C" {

struct SjLj_Function_Context;
void _Unwind_SjLj_Register(SjLj_Function_Context* fc);
void _Unwind_SjLj_Unregister(SjLj_Function_Context* fc);

} // extern "C"

namespace hamigaki { namespace coroutines { namespace detail {

struct sjlj_context
{
    sjlj_context* prev;
};

inline sjlj_context* replace_sjlj_context(sjlj_context* ctx)
{
    sjlj_context dummy;
    ::_Unwind_SjLj_Register(reinterpret_cast<SjLj_Function_Context*>(&dummy));
    sjlj_context* old = dummy.prev;
    dummy.prev = ctx;
    ::_Unwind_SjLj_Unregister(reinterpret_cast<SjLj_Function_Context*>(&dummy));
    return old;
}

} } } // End namespaces detail, coroutines, hamigaki.

#endif // HAMIGAKI_COROUTINE_DETAIL_GCC_SJLJ_CONTEXT_HPP
