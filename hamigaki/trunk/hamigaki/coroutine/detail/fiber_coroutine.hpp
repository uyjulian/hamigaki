//  fiber_coroutine.hpp: a simple coroutine class by using Win32 Fiber

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

#ifndef HAMIGAKI_COROUTINE_DETAIL_FIBER_COROUTINE_HPP
#define HAMIGAKI_COROUTINE_DETAIL_FIBER_COROUTINE_HPP

#include <hamigaki/coroutine/exception.hpp>
#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/coroutine/detail/fiber.hpp>
#else
    #error unsupported platform
#endif

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    #define HAMIGAKI_COROUTINE_BROKEN_BOOST_FUNCTION
    #include <hamigaki/coroutine/detail/borland/function1.hpp>
#else
    #include <boost/function.hpp>
#endif

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4355)
#endif

namespace hamigaki { namespace coroutine {

namespace coro_detail
{
    enum state { none, exiting, exited };
} // namespace coro_detail

template<class T>
class fiber_coroutine
{
public:
    class self;
    friend class self;

    class self
    {
    public:
        self(fiber_coroutine* c) : coro_(c) {}

        void yield(const T& t)
        {
            coro_->t_ = t;
            coro_->callee_.yield_to(coro_->caller_);
            if (coro_->state_ == coro_detail::exiting)
                throw exit_exception();
        }

#if defined(_MSC_VER) || defined(__BORLANDC__)
        __declspec(noreturn)
#endif
        void exit()
#if defined(__GNUC__)
        __attribute__((noreturn))
#endif
        {
            throw exit_exception();
        }

    private:
        fiber_coroutine* coro_;
    };

    template<class Functor>
    fiber_coroutine(std::size_t stack_size, Functor func)
        : func_(func), state_(coro_detail::none)
        , callee_(stack_size, startup, this)
    {
    }

    ~fiber_coroutine()
    {
        if (state_ != coro_detail::exited)
        {
            state_ = coro_detail::exiting;
            caller_.yield_to(callee_);
        }
    }

    void yield()
    {
        caller_.yield_to(callee_);
    }

    const T& result() const
    {
        return t_;
    }

    bool exited() const
    {
        return state_ == coro_detail::exited;
    }

private:
    T t_;
#if defined(HAMIGAKI_COROUTINE_BROKEN_BOOST_FUNCTION)
    detail::function1<T,self&> func_;
#else
    boost::function1<T,self&> func_;
#endif
    coro_detail::state state_;
    detail::fiber caller_;
    detail::fiber callee_;

    static void __stdcall startup(void* data)
    {
        fiber_coroutine* coro = static_cast<fiber_coroutine*>(data);
        self self(coro);
        try
        {
            coro->t_ = coro->func_(self);
            coro->callee_.yield_to(coro->caller_);
        }
        catch (...)
        {
        }
        coro->state_ = coro_detail::exited;
        coro->callee_.yield_to(coro->caller_);
    }
};

} } // End namespaces coroutine, hamigaki.

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#endif // HAMIGAKI_COROUTINE_DETAIL_FIBER_COROUTINE_HPP
