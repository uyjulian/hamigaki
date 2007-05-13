// ostream_iterator_test.cpp: test case for ostream_iterator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#include <hamigaki/iterator/ostream_iterator.hpp>
#include <boost/test/unit_test.hpp>
#include <sstream>

namespace ut = boost::unit_test;

void ostream_iterator_test()
{
    {
        std::ostringstream os;
        static_cast<void>(hamigaki::ostream_iterator<int>(os));
        BOOST_CHECK_EQUAL(os.str(), std::string());
    }

    {
        std::ostringstream os;
        hamigaki::ostream_iterator<int> i(os);
        *i = 0;
        ++i;
        BOOST_CHECK_EQUAL(os.str(), std::string("0"));
    }

    {
        std::ostringstream os;
        hamigaki::ostream_iterator<int> i(os);
        *i++ = 0;
        BOOST_CHECK_EQUAL(os.str(), std::string("0"));
    }

    {
        std::ostringstream os;
        hamigaki::ostream_iterator<int> i(os);
        *i++ = 0;
        *i++ = 1;
        BOOST_CHECK_EQUAL(os.str(), std::string("01"));
    }

    {
        std::ostringstream os;
        hamigaki::ostream_iterator<int>(os, ",");
        BOOST_CHECK_EQUAL(os.str(), std::string());
    }

    {
        std::ostringstream os;
        hamigaki::ostream_iterator<int> i(os, ",");
        *i = 0;
        ++i;
        BOOST_CHECK_EQUAL(os.str(), std::string("0"));
    }

    {
        std::ostringstream os;
        hamigaki::ostream_iterator<int> i(os, ",");
        *i++ = 0;
        BOOST_CHECK_EQUAL(os.str(), std::string("0"));
    }

    {
        std::ostringstream os;
        hamigaki::ostream_iterator<int> i(os, ",");
        *i++ = 0;
        *i++ = 1;
        BOOST_CHECK_EQUAL(os.str(), std::string("0,1"));
    }

    {
        std::ostringstream os;
        hamigaki::ostream_iterator<int> i(os, ",");
        *i++ = 0;
        *i++ = 1;
        *i++ = 2;
        BOOST_CHECK_EQUAL(os.str(), std::string("0,1,2"));
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("ostream_iterator test");
    test->add(BOOST_TEST_CASE(&ostream_iterator_test));
    return test;
}
