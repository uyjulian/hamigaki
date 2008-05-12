// byte_swap_test.cpp: test case for byte swap operations

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/integer for library home page.

#include <hamigaki/integer/byte_swap.hpp>
#include <boost/test/unit_test.hpp>

namespace ut = boost::unit_test;

void byte_swap16_test()
{
    BOOST_CHECK_EQUAL(hamigaki::byte_swap16(0x1234), 0x3412u);
}

void byte_swap32_test()
{
    BOOST_CHECK_EQUAL(hamigaki::byte_swap32(0x12345678), 0x78563412u);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("byte swap test");
    test->add(BOOST_TEST_CASE(&byte_swap16_test));
    test->add(BOOST_TEST_CASE(&byte_swap32_test));
    return test;
}
