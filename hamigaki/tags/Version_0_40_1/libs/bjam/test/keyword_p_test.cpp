// keyword_p_test.cpp: test case for keyword_p

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/util/keyword_parser.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam;
namespace ut = boost::unit_test;
using namespace boost::spirit;

void keyword_p_test()
{
    BOOST_CHECK(!parse("", bjam::keyword_p("if"), space_p).hit);
    BOOST_CHECK(parse("if", bjam::keyword_p("if"), space_p).hit);
    BOOST_CHECK(parse("if if", +bjam::keyword_p("if"), space_p).hit);
    BOOST_CHECK(!parse("ifif", +bjam::keyword_p("if"), space_p).hit);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("keyword_p test");
    test->add(BOOST_TEST_CASE(&keyword_p_test));
    return test;
}
