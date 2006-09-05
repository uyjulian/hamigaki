//  coroutine0.hpp: coroutine<R>

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#ifndef HAMIGAKI_COROUTINE_DETAIL_COROUTINE0_HPP
#define HAMIGAKI_COROUTINE_DETAIL_COROUTINE0_HPP

#include <boost/config.hpp>

#include <hamigaki/coroutine/detail/coroutine_utility.hpp>
#include <hamigaki/coroutine/detail/default_context.hpp>
#include <hamigaki/coroutine/exception.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <new>

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

namespace hamigaki { namespace coroutines {

template<class R, class ContextImpl=detail::default_context_impl>
class coroutine0 : private boost::noncopyable
{
public:
    class self;
    friend class self;

    class self
    {
    public:
        self(coroutine0* c) : coro_(c) {}

        void yield(const R& r)
        {
            coro_->result_ = r;
            swap_context(
                coro_->callee_, coro_->caller_, detail::default_hint());
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
        coroutine0* coro_;
    };

    template<class Functor>
    coroutine0(Functor func, std::ptrdiff_t stack_size=-1)
        : func_(func), func_storage_(this), state_(coro_detail::normal)
        , callee_(func_storage_, stack_size)
    {
    }

    ~coroutine0()
    {
        if (state_ != coro_detail::exited)
        {
            state_ = coro_detail::exiting;
            swap_context(caller_, callee_, detail::default_hint());
        }
    }

    R operator()()
    {
        swap_context(caller_, callee_, detail::default_hint());
        if (state_ == coro_detail::exited)
            throw coroutine_exited();
        return *result_;
    }

    boost::optional<R> operator()(const std::nothrow_t&)
    {
        swap_context(caller_, callee_, detail::default_hint());
        return result_;
    }

    bool exited() const
    {
        return state_ == coro_detail::exited;
    }

private:
    class functor;
    friend class functor;
    class functor
    {
    public:
        explicit functor(coroutine0* c) : coro_(c)
        {
        }

        void operator()() const
        {
            coro_->startup();
        }

    private:
        coroutine0* coro_;
    };

    boost::optional<R> result_;
#if defined(HAMIGAKI_COROUTINE_BROKEN_BOOST_FUNCTION)
    detail::function1<R,self&> func_;
#else
    boost::function1<R,self&> func_;
#endif
    functor func_storage_;
    coro_detail::state state_;
    typename ContextImpl::context_impl_base caller_;
    ContextImpl callee_;

    void startup()
    {
        self self(this);
        try
        {
            result_ = func_(self);
            swap_context(callee_, caller_, detail::default_hint());
        }
        catch (...)
        {
        }
        result_ = boost::none;
        state_ = coro_detail::exited;
        swap_context(callee_, caller_, detail::default_hint());
    }
};

} } // End namespaces coroutines, hamigaki.

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#undef HAMIGAKI_COROUTINE_BROKEN_BOOST_FUNCTION

#endif // HAMIGAKI_COROUTINE_DETAIL_COROUTINE0_HPP
