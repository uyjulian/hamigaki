// exit_other_test.cpp: test case for exit() from other thread

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

// This test is based on the code contributed by W.Dee.

#include <hamigaki/coroutine/coroutine.hpp>
#include <boost/test/unit_test.hpp>

namespace coro = hamigaki::coroutines;
namespace ut = boost::unit_test;

typedef coro::coroutine<void()> coroutine_type1;
typedef coro::coroutine<int(coroutine_type1*)> coroutine_type2;

void body1(coroutine_type1::self& self)
{
    while (true)
        self.yield();
}

int body2(coroutine_type2::self& self, coroutine_type1* coroutine1)
{
    coroutine1->exit();
    while (true)
        self.yield(0);
    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(0)
}

void exit_other_test()
{
    coroutine_type1 coroutine1(body1);
    coroutine1();

    coroutine_type2 coroutine2(body2);
    coroutine2(&coroutine1);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("exit other test");
    test->add(BOOST_TEST_CASE(&exit_other_test));
    return test;
}
