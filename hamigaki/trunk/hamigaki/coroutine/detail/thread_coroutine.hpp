//  thread_coroutine.hpp: a simple coroutine class by using Boost.Thread

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

#ifndef HAMIGAKI_COROUTINE_DETAIL_THREAD_COROUTINE_HPP
#define HAMIGAKI_COROUTINE_DETAIL_THREAD_COROUTINE_HPP

#include <hamigaki/coroutine/exception.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace hamigaki { namespace coroutine {

namespace th_coro_detail
{
    enum state { stopped, running, exiting, exited };
} // namespace th_coro_detail

template<class T>
class thread_coroutine
{
public:
    class self;
    friend class self;

    class self
    {
    public:
        self(thread_coroutine* c) : coro_(c) {}

        void yield(const T& t)
        {
            boost::mutex::scoped_lock locking(coro_->mutex_);
            coro_->t_ = t;
            coro_->state_ = th_coro_detail::stopped;
            coro_->cond_.notify_one();
            while (coro_->state_ == th_coro_detail::stopped)
                coro_->cond_.wait(locking);

            if (coro_->state_ == th_coro_detail::exiting)
                throw exit_exception();
        }

        void exit()
#if defined(__GNUC__)
        __attribute__((noreturn))
#endif
        {
            throw exit_exception();
        }

    private:
        thread_coroutine* coro_;
    };

    template<class Functor>
    thread_coroutine(std::size_t, Functor func)
        : func_(func), state_(th_coro_detail::stopped)
        , thread_(boost::bind(&thread_coroutine::startup, this))
    {
    }

    ~thread_coroutine()
    {
        {
            boost::mutex::scoped_lock locking(mutex_);
            if (state_ != th_coro_detail::exited)
            {
                state_ = th_coro_detail::exiting;
                cond_.notify_one();
                while (state_ != th_coro_detail::exited)
                    cond_.wait(locking);
            }
        }
        thread_.join();
    }

    void yield()
    {
        boost::mutex::scoped_lock locking(mutex_);
        state_ = th_coro_detail::running;
        cond_.notify_one();
        while (
            (state_ == th_coro_detail::running) ||
            (state_ == th_coro_detail::exiting))
        {
            cond_.wait(locking);
        }
    }

    const T& result() const
    {
        boost::mutex::scoped_lock locking(mutex_);
        return t_;
    }

    bool exited() const
    {
        boost::mutex::scoped_lock locking(mutex_);
        return state_ == th_coro_detail::exited;
    }

private:
    T t_;
    boost::function1<T,self&> func_;
    th_coro_detail::state state_;
    mutable boost::mutex mutex_;
    boost::condition cond_;
    boost::thread thread_;

    void startup()
    {
        self self(this);
        try
        {
            {
                boost::mutex::scoped_lock locking(mutex_);
                while (state_ == th_coro_detail::stopped)
                    cond_.wait(locking);
            }

            T tmp = func_(self);

            {
                boost::mutex::scoped_lock locking(mutex_);
                t_ = tmp;
                state_ = th_coro_detail::stopped;
                cond_.notify_one();
                while (state_ == th_coro_detail::stopped)
                    cond_.wait(locking);
            }
        }
        catch (...)
        {
        }

        boost::mutex::scoped_lock locking(mutex_);
        state_ = th_coro_detail::exited;
        cond_.notify_one();
    }
};

} } // End namespaces coroutine, hamigaki.

#endif // HAMIGAKI_COROUTINE_DETAIL_THREAD_COROUTINE_HPP
