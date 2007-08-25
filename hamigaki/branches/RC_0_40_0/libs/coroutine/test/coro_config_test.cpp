// coro_config_test.cpp: test case for the copy-ability of coroutine

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#include <hamigaki/coroutine/coroutine.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace coro = hamigaki::coroutines;
namespace ut = boost::unit_test;

void coro_config_test()
{
#if defined(HAMIGAKI_COROUTINE_DETAIL_FIBER_CONTEXT_HPP)
    std::cout << "use Win32 Fiber";
#elif defined(HAMIGAKI_COROUTINE_DETAIL_POSIX_USER_CONTEXT_HPP)
    std::cout << "use POSIX user context";
#elif defined(HAMIGAKI_COROUTINE_DETAIL_PTHREAD_CONTEXT_HPP)
    std::cout << "use POSIX thread";
#endif

#if defined(HAMIGAKI_COROUTINE_USE_SJLJ_CONTEXT)
    std::cout << " with SjLj-exceptions support";
#endif

    std::cout << std::endl;
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("coroutine config test");
    test->add(BOOST_TEST_CASE(&coro_config_test));
    return test;
}
