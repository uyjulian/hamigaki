// pthread_context.hpp: POSIX Thread based context implementation

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#ifndef HAMIGAKI_COROUTINE_DETAIL_PTHREAD_CONTEXT_HPP
#define HAMIGAKI_COROUTINE_DETAIL_PTHREAD_CONTEXT_HPP

#include <hamigaki/coroutine/detail/swap_context_hints.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <cstddef>
#include <pthread.h>

#if !defined(BOOST_DISABLE_ASSERTS) && \
    (defined(BOOST_ENABLE_ASSERT_HANDLER) || !defined(NDEBUG))

    #define HAMIGAKI_COROUTINE_DEBUG(x) x
#else
    #define HAMIGAKI_COROUTINE_DEBUG(x)
#endif

namespace hamigaki { namespace coroutines { namespace detail { namespace posix {

class condition;
class mutex : private boost::noncopyable
{
    friend class condition;

public:
    mutex()
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_mutex_init(&handle_, 0);
        BOOST_ASSERT(ret == 0);
    }

    ~mutex()
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_mutex_destroy(&handle_);
        BOOST_ASSERT(ret == 0);
    }

    void lock()
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_mutex_lock(&handle_);
        BOOST_ASSERT(ret == 0);
    }

    void unlock()
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_mutex_unlock(&handle_);
        BOOST_ASSERT(ret == 0);
    }

private:
    ::pthread_mutex_t handle_;
};

class condition : private boost::noncopyable
{
public:
    condition()
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_cond_init(&handle_, 0);
        BOOST_ASSERT(ret == 0);
    }

    ~condition()
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_cond_destroy(&handle_);
        BOOST_ASSERT(ret == 0);
    }

    void notify_one()
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_cond_signal(&handle_);
        BOOST_ASSERT(ret == 0);
    }

    void wait(mutex& m)
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_cond_wait(&handle_, &m.handle_);
        BOOST_ASSERT(ret == 0);
    }

private:
    ::pthread_cond_t handle_;
};

struct pthread_context_impl_data
{
    mutex mutex_;
    condition cond_;
    bool suspended_;
};

inline void mutex_unlock(void* p)
{
    mutex& m = *static_cast<mutex*>(p);
    m.unlock();
}

class pthread_context_impl_base
{
public:
    pthread_context_impl_base()
        : data_ptr_(new pthread_context_impl_data)
    {
        data_ptr_->suspended_ = false;
    }

    friend void swap_context(
        pthread_context_impl_base& from,
        const pthread_context_impl_base& to,
        default_hint)
    {
        from.data_ptr_->mutex_.lock();
        pthread_cleanup_push(&mutex_unlock, &from.data_ptr_->mutex_);

        from.data_ptr_->suspended_ = true;

        to.data_ptr_->mutex_.lock();
        to.data_ptr_->suspended_ = false;
        to.data_ptr_->cond_.notify_one();
        to.data_ptr_->mutex_.unlock();

        while (from.data_ptr_->suspended_)
        {
            from.data_ptr_->cond_.wait(from.data_ptr_->mutex_);
#if defined(__APPLE__)
            pthread_testcancel();
#endif
        }

        pthread_cleanup_pop(1);
    }

protected:
    boost::shared_ptr<pthread_context_impl_data> data_ptr_;
};

class pthread_context_impl
    : public pthread_context_impl_base
    , private boost::noncopyable
{
public:
    typedef pthread_context_impl_base context_impl_base;

    template<typename Functor>
    pthread_context_impl(Functor& f, std::ptrdiff_t)
        : func_ptr_(&f)
    {
        data_ptr_->suspended_ = true;
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_create(&handle_, 0, &trampoline<Functor>, this);
        BOOST_ASSERT(ret == 0);
    }

    ~pthread_context_impl()
    {
        HAMIGAKI_COROUTINE_DEBUG(int ret =)
            ::pthread_cancel(handle_);
        BOOST_ASSERT(ret == 0);

#if defined(__APPLE__)
        data_ptr_->mutex_.lock();
        data_ptr_->cond_.notify_one();
        data_ptr_->mutex_.unlock();
#endif

        HAMIGAKI_COROUTINE_DEBUG(ret =)
            ::pthread_join(handle_, 0);
        BOOST_ASSERT(ret == 0);
    }

private:
    ::pthread_t handle_;
    void* func_ptr_;

    template<typename T>
    static void* trampoline(void* p)
    {
        pthread_context_impl* this_ = static_cast<pthread_context_impl*>(p);
        this_->data_ptr_->mutex_.lock();
        while (this_->data_ptr_->suspended_)
            this_->data_ptr_->cond_.wait(this_->data_ptr_->mutex_);
        this_->data_ptr_->mutex_.unlock();

        T* func = static_cast<T*>(this_->func_ptr_);
        (*func)();

        return 0;
    }
};

typedef pthread_context_impl context_impl;

} } } } // End namespaces posix, detail, coroutines, hamigaki.

#undef HAMIGAKI_COROUTINE_DEBUG

#endif // HAMIGAKI_COROUTINE_DETAIL_PTHREAD_CONTEXT_HPP
