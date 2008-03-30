// restart_test.cpp: test case for restarting of coroutines

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#include <hamigaki/coroutine/coroutine.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace coro = hamigaki::coroutines;
namespace ut = boost::unit_test;

typedef coro::coroutine<int(void)> coroutine_type;

int count_body(coroutine_type::self& self)
{
    int i = 0;
    while (true)
        self.yield(i++);
}

void restart_test()
{
    coroutine_type c(count_body);

    BOOST_CHECK_EQUAL(c(), 0);
    BOOST_CHECK_EQUAL(c(), 1);
    BOOST_CHECK_EQUAL(c(), 2);

    c.restart();

    BOOST_CHECK_EQUAL(c(), 0);
    BOOST_CHECK_EQUAL(c(), 1);
    BOOST_CHECK_EQUAL(c(), 2);

    c.exit();
    c.restart();

    BOOST_CHECK_EQUAL(c(), 0);
    BOOST_CHECK_EQUAL(c(), 1);
    BOOST_CHECK_EQUAL(c(), 2);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("restart test");
    test->add(BOOST_TEST_CASE(&restart_test));
    return test;
}
