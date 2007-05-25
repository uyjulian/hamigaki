// expand_variable_test.cpp: test case for expand_variable

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/util/variable_expansion.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam;
namespace ut = boost::unit_test;

void simple_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("hoge"));
    bjam::list_of_list args;

    bjam::list_type result;
    bjam::list_type expect;

    bjam::expand_variable(result, "", table, args);
    expect = boost::assign::list_of("");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("abc");
    bjam::expand_variable(result, "abc", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("hoge");
    bjam::expand_variable(result, "$(X)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void index_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("a")("b")("c"));
    bjam::list_of_list args;

    bjam::list_type result;
    bjam::list_type expect;

    expect = boost::assign::list_of("a");
    bjam::expand_variable(result, "$(X[1])", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("b")("c");
    bjam::expand_variable(result, "$(X[2-3])", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("b")("c");
    bjam::expand_variable(result, "$(X[2-])", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("c");
    bjam::expand_variable(result, "$(X[-1])", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void modifiers_test()
{
    bjam::variable_table table;
    table.set_values("A", boost::assign::list_of("aBcD"));
    table.set_values("X", boost::assign::list_of("<g>/d/b.s(m)"));
    bjam::list_of_list args;

    bjam::list_type result;
    bjam::list_type expect;

    expect = boost::assign::list_of("abcd");
    bjam::expand_variable(result, "$(A:L)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("ABCD");
    bjam::expand_variable(result, "$(A:U)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

// TODO:
#if 0
    result.clear();
    expect = boost::assign::list_of("hoge");
    bjam::expand_variable(result, "$(:E=hoge)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
#endif

    result.clear();
    expect = boost::assign::list_of("b.s");
    bjam::expand_variable(result, "$(X:BS)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("<g>/d/b.txt(m)");
    bjam::expand_variable(result, "$(X:S=.txt)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("expand_variable test");
    test->add(BOOST_TEST_CASE(&simple_test));
    test->add(BOOST_TEST_CASE(&index_test));
    test->add(BOOST_TEST_CASE(&modifiers_test));
    return test;
}
