//  coroutine_test.cpp: test case for coroutine

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#include <hamigaki/coroutine/coroutine.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace coro = hamigaki::coroutines;
namespace ut = boost::unit_test;

typedef coro::coroutine<int(int)> coroutine_type;
typedef coro::coroutine<int(int,int)> coroutine_type2;

int twice_body(coroutine_type::self& self, int n)
{
    while (true)
        n = self.yield(n*2);
}

int add_body(coroutine_type2::self& self, int a, int b)
{
    while (true)
        boost::tie(a, b) = self.yield(a+b);
}

void coroutine_test()
{
    coroutine_type twice(twice_body);
    for (int i = 0; i < 10; ++i)
        std::cout << twice(i) << '\n';
    std::cout << std::endl;

    coroutine_type2 add(add_body);
    for (int j = 0; j < 3; ++j)
        for (int i = 0; i < 3; ++i)
            std::cout << add(i,j) << '\n';
    std::cout << std::endl;
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("coroutine test");
    test->add(BOOST_TEST_CASE(&coroutine_test));
    return test;
}
