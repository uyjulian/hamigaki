// processor_test.cpp: test case for processor

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#include <hamigaki/coroutine/processor.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <iostream>

namespace coro = hamigaki::coroutines;
namespace ut = boost::unit_test;

typedef coro::processor<int> processor_type;

void print_processor_body(processor_type::self& self, int arg)
{
    while (true)
    {
        std::cout << arg << std::endl;
        arg = self.yield();
    }
}

void processor_test()
{
    std::copy(
        boost::make_counting_iterator(0),
        boost::make_counting_iterator(10),
        processor_type(print_processor_body)
    );
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("processor test");
    test->add(BOOST_TEST_CASE(&processor_test));
    return test;
}
