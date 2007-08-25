// coroutine_test.cpp: test case for coroutine

// Copyright Takeshi Mouri 2006, 2007.
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

typedef std::string my_string; // CodeWarrior workaround

typedef coro::coroutine<void(void)> coroutine_type0;
typedef coro::coroutine<int(int)> coroutine_type;
typedef coro::coroutine<int(int,int)> coroutine_type2;
typedef coro::coroutine<std::string(my_string,int,int)> coroutine_type3;

void hello_body(coroutine_type0::self& self)
{
    while (true)
    {
        std::cout << "hello\n";
        self.yield();
    }
}

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

std::string append_number_body(
    coroutine_type3::self& self, std::string s, int a, int b)
{
    while (true)
    {
        std::ostringstream os;
        os << s << a << b;
        boost::tie(s, a, b) = self.yield(os.str());
    }
}

void coroutine_test()
{
    coroutine_type0 hello(hello_body);
    for (int i = 0; i < 3; ++i)
        hello();
    std::cout << std::endl;

    coroutine_type twice(twice_body);
    for (int i = 0; i < 10; ++i)
        std::cout << twice(i) << '\n';
    std::cout << std::endl;

    coroutine_type2 add(add_body);
    for (int j = 0; j < 3; ++j)
        for (int i = 0; i < 3; ++i)
            std::cout << add(i,j) << '\n';
    std::cout << std::endl;

    coroutine_type3 append(append_number_body);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            std::cout << append("s", i, j) << '\n';
    std::cout << std::endl;
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("coroutine test");
    test->add(BOOST_TEST_CASE(&coroutine_test));
    return test;
}
