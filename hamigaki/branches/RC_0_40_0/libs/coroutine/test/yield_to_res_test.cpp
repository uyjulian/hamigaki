// yield_to_res_test.cpp: test case for yield_to()

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

typedef coro::shared_coroutine<int(void)> coroutine_type;

coroutine_type coro_a;
coroutine_type coro_b;
coroutine_type coro_c;

int a_body(coroutine_type::self& self)
{
    std::cout << "A" << std::endl;
    self.yield_to(coro_b);

    std::cout << "A" << std::endl;
    self.yield_to(coro_b);

    std::cout << "A" << std::endl;
    self.yield(2);

    std::cout << "A" << std::endl;
    self.yield_to(coro_b);

    BOOST_FAIL("Coroutine-A reached the end of the function");
    self.exit();
    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(0)
}

int b_body(coroutine_type::self& self)
{
    std::cout << "B" << std::endl;
    self.yield(1);

    std::cout << "B" << std::endl;
    self.yield_to(coro_a);

    std::cout << "B" << std::endl;
    self.yield_to(coro_c);

    BOOST_FAIL("Coroutine-B reached the end of the function");
    self.exit();
    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(0)
}

int c_body(coroutine_type::self& self)
{
    std::cout << "C" << std::endl;
    self.yield(3);

    BOOST_FAIL("Coroutine-C reached the end of the function");
    self.exit();
    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(0)
}

void yield_to_res_test()
{
    coro_a = coroutine_type(&a_body);
    coro_b = coroutine_type(&b_body);
    coro_c = coroutine_type(&c_body);

    BOOST_CHECK_EQUAL(coro_a(), 1);
    std::cout << std::endl;

    BOOST_CHECK_EQUAL(coro_a(), 2);
    std::cout << std::endl;

    BOOST_CHECK_EQUAL(coro_a(), 3);
    std::cout << std::endl;

    std::cout << "OK" << std::endl;
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test =
        BOOST_TEST_SUITE("yield_to() test with the result");
    test->add(BOOST_TEST_CASE(&yield_to_res_test));
    return test;
}
