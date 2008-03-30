// counting_output_iter_test.cpp: test case for counting_output_iterator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#include <hamigaki/iterator/counting_output_iterator.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>

namespace ut = boost::unit_test;

void counting_output_iter_test()
{
    BOOST_CHECK_EQUAL(hamigaki::counting_output_iterator<int>().base(), 0);

    BOOST_CHECK_EQUAL(
        hamigaki::counting_output_iterator<int>(1).base(), 1);

    {
        hamigaki::counting_output_iterator<int> i;
        BOOST_CHECK_EQUAL(i.base(), 0);
        ++i;
        BOOST_CHECK_EQUAL(i.base(), 1);
    }

    {
        hamigaki::counting_output_iterator<int> i(1);
        BOOST_CHECK_EQUAL(i.base(), 1);
        ++i;
        BOOST_CHECK_EQUAL(i.base(), 2);
    }

    BOOST_CHECK_EQUAL(
        std::copy(
            boost::make_counting_iterator(0),
            boost::make_counting_iterator(10),
            hamigaki::counting_output_iterator<int>()
        ).base(),
        10
    );

    BOOST_CHECK_EQUAL(
        std::copy(
            boost::make_counting_iterator(0),
            boost::make_counting_iterator(10),
            hamigaki::make_counting_output_iterator(1)
        ).base(),
        11
    );
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("counting_output_iterator test");
    test->add(BOOST_TEST_CASE(&counting_output_iter_test));
    return test;
}
