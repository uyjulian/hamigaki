//  exception.hpp: the exception classes for Hamigaki.Coroutine

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

#ifndef HAMIGAKI_COROUTINE_EXCEPTION_HPP
#define HAMIGAKI_COROUTINE_EXCEPTION_HPP

#include <exception>

namespace hamigaki { namespace coroutine {

class exit_exception:  public std::exception {};

} } // End namespaces coroutine, hamigaki.

#endif // HAMIGAKI_COROUTINE_EXCEPTION_HPP
