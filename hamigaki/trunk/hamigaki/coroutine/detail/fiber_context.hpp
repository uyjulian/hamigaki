// fiber_context.hpp: Win32 fiber based context implementation

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

// Original Copyright
// ===========================================================================>
//  Copyright (c) 2006, Giovanni P. Deretta
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
// <===========================================================================

#ifndef HAMIGAKI_COROUTINE_DETAIL_FIBER_CONTEXT_HPP
#define HAMIGAKI_COROUTINE_DETAIL_FIBER_CONTEXT_HPP

#include <hamigaki/coroutine/detail/windows/fiber.hpp>
#include <hamigaki/coroutine/detail/swap_context_hints.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <cstddef>

#if !defined(BOOST_DISABLE_ASSERTS) && \
    (defined(BOOST_ENABLE_ASSERT_HANDLER) || !defined(NDEBUG))

    #define HAMIGAKI_COROUTINE_DEBUG(x) x
#else
    #define HAMIGAKI_COROUTINE_DEBUG(x)
#endif

#if defined(__GNUC__) && defined(__USING_SJLJ_EXCEPTIONS__)
    #define HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT
    #include <hamigaki/coroutine/detail/gcc/sjlj_context.hpp>
#endif

namespace hamigaki { namespace coroutines { namespace detail {

namespace windows
{

class fiber_context_impl_base
{
public:
    fiber_context_impl_base()
        : context_(0)
#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
        , eh_ctx_(0)
#endif
    {
    }

    friend void swap_context(
        fiber_context_impl_base& from,
        const fiber_context_impl_base& to,
        default_hint)
    {
        if (is_fiber())
        {
            bool need_reset = false;
            if (from.context_ == 0)
            {
                from.context_ = get_current_fiber();
                need_reset = true;
            }
            from.switch_to(to);
            if (need_reset)
                from.context_ = 0;
        }
        else
        {
            from.context_ = ::ConvertThreadToFiber(0);
            from.switch_to(to);
            from.context_ = 0;
        }
    }

protected:
    void* context_;

private:
#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
    detail::sjlj_context* eh_ctx_;
#endif

    void switch_to(const fiber_context_impl_base& f)
    {
#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
        eh_ctx_ = detail::replace_sjlj_context(f.eh_ctx_);
#endif
        ::SwitchToFiber(f.context_);
    }
};

class fiber_context_impl
    : public fiber_context_impl_base
    , private boost::noncopyable
{
public:
    typedef fiber_context_impl_base context_impl_base;

    template<class Functor>
    fiber_context_impl(Functor& f, std::ptrdiff_t stack_size)
    {
        if (stack_size == -1)
            stack_size = 0;
        context_ = ::CreateFiber(
            static_cast<unsigned long>(stack_size), &trampoline<Functor>, &f);
        BOOST_ASSERT(context_ != 0);
    }

    ~fiber_context_impl()
    {
        ::DeleteFiber(context_);
    }

private:
    template<typename T>
    static void __stdcall trampoline(void* p)
    {
        T* func = static_cast<T*>(p);
        (*func)();
    }
};

typedef fiber_context_impl context_impl;

} // namespace windows

} } } // End namespaces detail, coroutines, hamigaki.

#endif // HAMIGAKI_COROUTINE_DETAIL_FIBER_CONTEXT_HPP
