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

#include <hamigaki/coroutine/detail/coroutine0.hpp>
#include <hamigaki/coroutine/detail/coroutine1.hpp>
#include <hamigaki/coroutine/detail/coroutine2.hpp>

namespace hamigaki { namespace coroutines {

template<class Signature>
class coroutine;

template<class R>
class coroutine<R(void)> : public coroutine0<R>
{
public:
    template<class Functor>
    coroutine(Functor func, std::ptrdiff_t stack_size=-1)
        : coroutine0<R>(func, stack_size)
    {
    }
};

template<class R, class T1>
class coroutine<R(T1)> : public coroutine1<R,T1>
{
public:
    template<class Functor>
    coroutine(Functor func, std::ptrdiff_t stack_size=-1)
        : coroutine1<R,T1>(func, stack_size)
    {
    }
};

template<class R, class T1, class T2>
class coroutine<R(T1,T2)> : public coroutine2<R,T1,T2>
{
public:
    template<class Functor>
    coroutine(Functor func, std::ptrdiff_t stack_size=-1)
        : coroutine2<R,T1,T2>(func, stack_size)
    {
    }
};

} } // End namespaces coroutines, hamigaki.

#endif // HAMIGAKI_COROUTINE_COROUTINE_HPP
