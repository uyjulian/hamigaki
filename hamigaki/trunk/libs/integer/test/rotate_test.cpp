// rotate_test.cpp: test case for rotate operations

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/integer for library home page.

#include <hamigaki/integer/rotate.hpp>
#include <boost/test/unit_test.hpp>

namespace ut = boost::unit_test;

void rotate_left_test()
{
    BOOST_CHECK_EQUAL(hamigaki::rotate_left(0x12345678, 4), 0x23456781u);
}

void rotate_right_test()
{
    BOOST_CHECK_EQUAL(hamigaki::rotate_right(0x12345678, 4), 0x81234567u);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("rotate test");
    test->add(BOOST_TEST_CASE(&rotate_left_test));
    test->add(BOOST_TEST_CASE(&rotate_right_test));
    return test;
}
