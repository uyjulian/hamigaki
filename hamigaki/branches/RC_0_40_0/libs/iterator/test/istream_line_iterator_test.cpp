// istream_line_iterator_test.cpp: test case for istream_line_iterator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#include <hamigaki/iterator/istream_line_iterator.hpp>
#include <boost/test/unit_test.hpp>
#include <sstream>

namespace ut = boost::unit_test;

void istream_line_iterator_test()
{
    typedef hamigaki::istream_line_iterator<char> iter_type;

    BOOST_CHECK(iter_type() == iter_type());

    {
        std::istringstream is;
        BOOST_CHECK(iter_type(is) == iter_type());
    }

    {
        std::istringstream is("a");
        iter_type i(is);
        BOOST_CHECK(i != iter_type());
        BOOST_CHECK_EQUAL(*i, std::string("a"));
        ++i;
        BOOST_CHECK(i == iter_type());
    }

    {
        std::istringstream is("\n");
        iter_type i(is);
        BOOST_CHECK(i != iter_type());
        BOOST_CHECK_EQUAL(*i, std::string());
        ++i;
        BOOST_CHECK(i == iter_type());
    }

    {
        std::istringstream is(",a");
        iter_type i(is, ',');
        BOOST_CHECK(i != iter_type());
        BOOST_CHECK_EQUAL(*i, std::string());
        ++i;
        BOOST_CHECK(i != iter_type());
        BOOST_CHECK_EQUAL(*i, std::string("a"));
        ++i;
        BOOST_CHECK(i == iter_type());
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("istream_line_iterator test");
    test->add(BOOST_TEST_CASE(&istream_line_iterator_test));
    return test;
}
