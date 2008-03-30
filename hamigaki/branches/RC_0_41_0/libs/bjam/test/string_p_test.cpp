// string_p_test.cpp: test case for string_p

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/util/string_parser.hpp>
#include <boost/spirit/actor/assign_actor.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam;
namespace ut = boost::unit_test;
using namespace boost::spirit;

void string_p_test()
{
    BOOST_CHECK(!parse("", bjam::string_p).hit);
    BOOST_CHECK(parse("}", bjam::string_p >> '}').full);
    BOOST_CHECK(parse("foo}", bjam::string_p >> '}').full);
    BOOST_CHECK(parse(" foo }", bjam::string_p >> '}').full);
    BOOST_CHECK(parse("foo{}}", bjam::string_p >> '}').full);
    BOOST_CHECK(parse("foo{bar}}", bjam::string_p >> '}').full);
    BOOST_CHECK(parse("foo{}bar}", bjam::string_p >> '}').full);
    BOOST_CHECK(!parse("foo{}", bjam::string_p >> '}').full);

    // check CR/LF conversion
    BOOST_CHECK(parse("\r\n}", bjam::string_p >> '}').full);
    BOOST_CHECK(parse("\n}", bjam::string_p >> '}').full);
}

void action_test()
{
    std::string result;

    BOOST_REQUIRE(parse("foo}", bjam::string_p[assign_a(result)] >> '}').full);
    BOOST_CHECK_EQUAL(result, "foo");

    BOOST_REQUIRE(
        parse(" foo }", bjam::string_p[assign_a(result)] >> '}').full);
    BOOST_CHECK_EQUAL(result, " foo ");

    BOOST_REQUIRE(
        parse("foo{}}", bjam::string_p[assign_a(result)] >> '}').full);
    BOOST_CHECK_EQUAL(result, "foo{}");

    // check CR/LF conversion
    BOOST_REQUIRE(parse("\r\n}", bjam::string_p[assign_a(result)] >> '}').full);
    BOOST_CHECK_EQUAL(result, "\n");
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("string_p test");
    test->add(BOOST_TEST_CASE(&string_p_test));
    test->add(BOOST_TEST_CASE(&action_test));
    return test;
}
