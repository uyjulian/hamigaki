//  fiber.hpp: fiber wrapper class

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

// Original Copyright
// ===========================================================================>
//  Copyright (c) 2006, Giovanni P. Deretta
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
// <===========================================================================

#ifndef HAMIGAKI_COROUTINE_DETAIL_FIBER_HPP
#define HAMIGAKI_COROUTINE_DETAIL_FIBER_HPP

#include <boost/noncopyable.hpp>
#include <cstddef>
#include <stddef.h>

#if defined(__GNUC__) && defined(__USING_SJLJ_EXCEPTIONS__)
    #define HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT
    #include <hamigaki/coroutine/detail/gcc/sjlj_context.hpp>
#endif

#if defined(__CYGWIN__)
    #define HAMIGAKI_COROUTINE_USE_CYG_TLS
    #include <hamigaki/coroutine/detail/cygwin/cyg_tls.hpp>
#endif

#if defined(_MSC_FULL_VER) && (_MSC_FULL_VER >= 13012035)
    #define HAMIGAKI_COROUTINE_HAS_READFSDWORD
    extern "C" unsigned long __readfsdword(unsigned long);
    #pragma intrinsic(__readfsdword)
#endif

extern "C" {

typedef void (__stdcall* start_routine)(void*);

__declspec(dllimport)
void* __stdcall CreateFiber(unsigned long, start_routine, void*);

__declspec(dllimport) void __stdcall DeleteFiber(void*);
__declspec(dllimport) void* __stdcall ConvertThreadToFiber(void*);
__declspec(dllimport) void __stdcall SwitchToFiber(void*);

} // extern "C"

namespace hamigaki { namespace coroutine { namespace detail {

#if defined(HAMIGAKI_COROUTINE_HAS_READFSDWORD)
inline void* get_current_fiber()
{
    return reinterpret_cast<void*>(
        static_cast< ::uintptr_t>(::__readfsdword(0x10))
    );
}
#elif (defined(_MSC_VER) || defined(__MWERKS__)) && defined(_M_IX86)
inline void* get_current_fiber()
{
    __asm mov eax, fs:[0x10]
}
#elif defined(__BORLANDC__) && defined(_M_IX86)
inline void* get_current_fiber()
{
    // mov eax, fs:[0x10]
    __emit__(0x64, 0xA1, 0x10, 0x00, 0x00, 0x00);

    return reinterpret_cast<void*>(_EAX);
}
#elif defined(__GNUC__) && defined(__i386__)
inline void* get_current_fiber()
{
    void* result;
    __asm__
    (
        "movl %%fs:0x10,%0"
        : "=r" (result)
    );
    return result;
}
#else
    #error unsupported platform
#endif

inline bool is_fiber()
{
    void* p = get_current_fiber();
    return (p != 0) && (p != reinterpret_cast<void*>(0x00001E00));
}

class fiber : private boost::noncopyable
{
public:
    fiber() : context_(0), delete_on_exit_(false)
    {
    }

    fiber(std::size_t stack_size, start_routine func, void* data)
        : context_(::CreateFiber(stack_size, func, data))
        , delete_on_exit_(true)
#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
        , eh_ctx_(0)
#endif
    {
    }

    ~fiber()
    {
        if (delete_on_exit_)
            ::DeleteFiber(context_);
    }

    void yield_to(fiber& f)
    {
        if (is_fiber())
        {
            if (context_ == 0)
                context_ = get_current_fiber();
            switch_to(f);
        }
        else
        {
            context_ = ::ConvertThreadToFiber(0);
            switch_to(f);
        }
    }

#if defined(HAMIGAKI_COROUTINE_USE_CYG_TLS)
    void copy_cyg_tls_from(fiber& f)
    {
        copy_cyg_tls_first(context_, f.context_);
    }
#endif

private:
    void* context_;
    bool delete_on_exit_;
#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
    detail::sjlj_context* eh_ctx_;
#endif

    void switch_to(fiber& f)
    {
#if defined(HAMIGAKI_COROUTINE_USE_CYG_TLS)
        copy_cyg_tls(f.context_, context_);
#endif
#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
        eh_ctx_ = detail::replace_sjlj_context(f.eh_ctx_);
#endif
        ::SwitchToFiber(f.context_);
    }
};

} } } // End namespaces detail, coroutine, hamigaki.

#endif // HAMIGAKI_COROUTINE_DETAIL_FIBER_HPP
