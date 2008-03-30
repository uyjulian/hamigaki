// coroutine_utility.hpp: utilities for coroutine

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#ifndef HAMIGAKI_COROUTINE_DETAIL_COROUTINE_UTILITY_HPP
#define HAMIGAKI_COROUTINE_DETAIL_COROUTINE_UTILITY_HPP

#if defined(_MSC_VER) || defined(__GNUC__)
    #define HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(result)
#else
    #define HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(result) return (result);
#endif

namespace hamigaki { namespace coroutines {

namespace coro_detail
{
    enum state { normal, exiting, exited };
} // namespace coro_detail

} } // End namespaces coroutines, hamigaki.

#endif // HAMIGAKI_COROUTINE_DETAIL_COROUTINE_UTILITY_HPP
