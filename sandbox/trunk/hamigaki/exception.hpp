// exception.hpp: Hamigaki.Exception root header file

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HAMIGAKI_EXCEPTION_HPP
#define HAMIGAKI_EXCEPTION_HPP

#include <boost/config.hpp>

#if defined(__GNUC__)
    #include <hamigaki/exception/gcc/exception.hpp>
#elif defined(BOOST_MSVC)
    #include <hamigaki/exception/msvc/exception.hpp>
#else
    #error "Sorry, unsupported compiler"
#endif

#endif // HAMIGAKI_EXCEPTION_HPP
