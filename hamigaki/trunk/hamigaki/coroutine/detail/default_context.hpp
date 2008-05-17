// default_context.hpp: default context implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#ifndef HAMIGAKI_COROUTINE_DETAIL_DEFAULT_CONTEXT_HPP
#define HAMIGAKI_COROUTINE_DETAIL_DEFAULT_CONTEXT_HPP

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
#include <hamigaki/coroutine/detail/fiber_context.hpp>
namespace hamigaki { namespace coroutines { namespace detail {
    typedef windows::fiber_context_impl default_context_impl;
} } } // End namespaces detail, coroutines, hamigaki.
#elif (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)) || defined(__NetBSD__)
#include <hamigaki/coroutine/detail/posix_user_context.hpp>
namespace hamigaki { namespace coroutines { namespace detail {
    typedef posix::user_context_impl default_context_impl;
} } } // End namespaces detail, coroutines, hamigaki.
#elif defined(BOOST_HAS_PTHREADS)
#include <hamigaki/coroutine/detail/pthread_context.hpp>
namespace hamigaki { namespace coroutines { namespace detail {
    typedef posix::pthread_context_impl default_context_impl;
} } } // End namespaces detail, coroutines, hamigaki.
#else
    #error unsupported platform
#endif

#endif // HAMIGAKI_COROUTINE_DETAIL_DEFAULT_CONTEXT_HPP
