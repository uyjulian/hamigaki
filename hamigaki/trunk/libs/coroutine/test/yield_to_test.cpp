// yield_to_test.cpp: test case for yield_to()

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#include <hamigaki/coroutine/shared_coroutine.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace coro = hamigaki::coroutines;
namespace ut = boost::unit_test;

typedef coro::shared_coroutine<void(void)> coroutine_type;

coroutine_type coro_a;
coroutine_type coro_b;

void a_body(coroutine_type::self& self)
{
    std::cout << "A" << std::endl;
    self.yield_to(coro_b);

    std::cout << "A" << std::endl;
    self.yield_to(coro_b);

    std::cout << "A" << std::endl;
    self.yield();

    BOOST_FAIL("Coroutine-A reached the end of the function");
}

void b_body(coroutine_type::self& self)
{
    std::cout << "B" << std::endl;
    self.yield();

    std::cout << "B" << std::endl;
    self.yield_to(coro_a);

    BOOST_FAIL("Coroutine-B reached the end of the function");
}

void yield_to_test()
{
    coro_a = coroutine_type(&a_body);
    coro_b = coroutine_type(&b_body);

    coro_a();
    std::cout << std::endl;

    coro_a();
    std::cout << std::endl;

    std::cout << "OK" << std::endl;
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("yield_to() test");
    test->add(BOOST_TEST_CASE(&yield_to_test));
    return test;
}
