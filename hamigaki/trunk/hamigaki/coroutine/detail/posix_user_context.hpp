// posix_user_context.hpp: POSIX ucontext based context implementation

// Copyright Takeshi Mouri 2006-2008.
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

#ifndef HAMIGAKI_COROUTINE_DETAIL_POSIX_USER_CONTEXT_HPP
#define HAMIGAKI_COROUTINE_DETAIL_POSIX_USER_CONTEXT_HPP

#include <hamigaki/coroutine/detail/swap_context_hints.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <cstddef>
#include <ucontext.h>

#include <hamigaki/detail/virtual_memory.hpp>

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

namespace posix
{

class user_context_impl_base
{
public:
    user_context_impl_base()
        : context_(new ::ucontext_t)
#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
        , eh_ctx_(0)
#endif
    {
    }

    friend void swap_context(
        user_context_impl_base& from,
        const user_context_impl_base& to,
        default_hint)
    {
#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
        from.eh_ctx_ = detail::replace_sjlj_context(to.eh_ctx_);
#endif
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
        ::swapcontext(from.context_.get(), to.context_.get());
        BOOST_ASSERT(ret == 0);
    }

protected:
    boost::shared_ptr< ::ucontext_t> context_;

private:
#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
    detail::sjlj_context* eh_ctx_;
#endif
};

class user_context_impl
    : public user_context_impl_base
    , private boost::noncopyable
{
public:
    typedef user_context_impl_base context_impl_base;

    static std::ptrdiff_t fix_stack_size(std::ptrdiff_t stack_size)
    {
        return stack_size == -1 ? 65536 : stack_size;
    }

    template<class Functor>
    user_context_impl(Functor& f, std::ptrdiff_t stack_size)
        : stack_(fix_stack_size(stack_size), PROT_READ|PROT_WRITE|PROT_EXEC)
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
        ::getcontext(context_.get());
        BOOST_ASSERT(ret == 0);

        context_->uc_stack.ss_sp = stack_.address();
        context_->uc_stack.ss_size = fix_stack_size(stack_size);
        context_->uc_link = 0;

        typedef void (*trampoline_pointer)(void*);
        trampoline_pointer tp = &trampoline<Functor>;

        ::makecontext(context_.get(), reinterpret_cast<void (*)()>(tp), 1, &f);
        BOOST_ASSERT(ret == 0);
    }

private:
    hamigaki::detail::posix::virtual_memory stack_;

    template<typename T>
    static void trampoline(void* p)
    {
        T* func = static_cast<T*>(p);
        (*func)();
    }
};

typedef user_context_impl context_impl;

} // namespace posix

} } } // End namespaces detail, coroutines, hamigaki.

#undef HAMIGAKI_COROUTINE_DEBUG

#endif // HAMIGAKI_COROUTINE_DETAIL_POSIX_USER_CONTEXT_HPP
