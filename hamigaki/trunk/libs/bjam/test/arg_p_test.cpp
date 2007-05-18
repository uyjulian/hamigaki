// arg_p_test.cpp: test case for arg_p

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/util/argument_parser.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam;
namespace ut = boost::unit_test;
using namespace boost::spirit;

void arg_p_test()
{
    BOOST_CHECK(!parse("", bjam::arg_p, space_p).hit);
    BOOST_CHECK(parse("foo", bjam::arg_p, space_p).full);
    BOOST_CHECK(!parse("if", bjam::arg_p, space_p).full);
    BOOST_CHECK(!parse(":", bjam::arg_p, space_p).full);
    BOOST_CHECK(parse("ifelse", bjam::arg_p, space_p).full);
    BOOST_CHECK(parse("foo bar", +bjam::arg_p, space_p).full);
    BOOST_CHECK(!parse("if else", +bjam::arg_p, space_p).full);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("arg_p test");
    test->add(BOOST_TEST_CASE(&arg_p_test));
    return test;
}
