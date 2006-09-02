//  coroutine.hpp: a simple coroutine class

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

#ifndef HAMIGAKI_COROUTINE_COROUTINE_HPP
#define HAMIGAKI_COROUTINE_COROUTINE_HPP

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/coroutine/detail/fiber_coroutine.hpp>
#else
    #include <hamigaki/coroutine/detail/thread_coroutine.hpp>
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
    #define HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(result)
#else
    #define HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(result) return (result);
#endif

namespace hamigaki { namespace coroutine {

template<class T>
class coroutine
#if defined(BOOST_WINDOWS)
    : public ::hamigaki::coroutine::fiber_coroutine<T>
#else
    : public ::hamigaki::coroutine::thread_coroutine<T>
#endif
{
#if defined(BOOST_WINDOWS)
    typedef ::hamigaki::coroutine::fiber_coroutine<T> impl_type;
#else
    typedef ::hamigaki::coroutine::thread_coroutine<T> impl_type;
#endif

public:
    typedef typename impl_type::self self;

    template<class Functor>
    coroutine(std::size_t stack_size, Functor func)
        : impl_type(stack_size, func)
    {
    }
};

} } // End namespaces coroutine, hamigaki.

#endif // HAMIGAKI_COROUTINE_COROUTINE_HPP
