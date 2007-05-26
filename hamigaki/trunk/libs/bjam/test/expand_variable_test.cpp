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

void case_conv_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("aBcD"));
    bjam::list_of_list args;

    bjam::list_type result;
    bjam::list_type expect;

    expect = boost::assign::list_of("abcd");
    bjam::expand_variable(result, "$(X:L)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("ABCD");
    bjam::expand_variable(result, "$(X:U)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("ABCD");
    bjam::expand_variable(result, "$(X:UL)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void empty_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("a")("b")("c"));
    bjam::list_of_list args;

    bjam::list_type result;
    bjam::list_type expect;

    expect = boost::assign::list_of("hoge");
    bjam::expand_variable(result, "$(:E=hoge)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("a")("b")("c");
    bjam::expand_variable(result, "$(:E=$(X))", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("a")("b")("c");
    bjam::expand_variable(result, "$([1]:E=$(X))", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void join_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("a")("b")("c"));
    table.set_values("Y", boost::assign::list_of(".")("-"));

    bjam::list_of_list args;

    bjam::list_type result;
    bjam::list_type expect;

    expect = boost::assign::list_of("a_b_c");
    bjam::expand_variable(result, "$(X:J=_)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    // Note: This may be a bjam bug
    result.clear();
    expect = boost::assign::list_of("a-b-c");
    bjam::expand_variable(result, "$(X:J=$(Y))", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void path_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("<g>/d/b.s(m)"));
    table.set_values("SYS", boost::assign::list_of("C:\\Windows\\System32"));
    table.set_values("TMP", boost::assign::list_of("/tmp"));
    bjam::list_of_list args;

    bjam::list_type result;
    bjam::list_type expect;

    expect = boost::assign::list_of("b.s");
    bjam::expand_variable(result, "$(X:BS)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("<g>/d/b.txt(m)");
    bjam::expand_variable(result, "$(X:S=.txt)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("<g>/d");
    bjam::expand_variable(result, "$(X:P)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("C:/Windows/System32");
    bjam::expand_variable(result, "$(SYS:T)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
#if defined(__CYGWIN__)
    // FIXME: Cygwin may not be installed to the default location
    expect = boost::assign::list_of("C:\\cygwin\\tmp");
#else
    expect = boost::assign::list_of("/tmp");
#endif
    bjam::expand_variable(result, "$(TMP:W)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("expand_variable test");
    test->add(BOOST_TEST_CASE(&simple_test));
    test->add(BOOST_TEST_CASE(&index_test));
    test->add(BOOST_TEST_CASE(&case_conv_test));
    test->add(BOOST_TEST_CASE(&empty_test));
    test->add(BOOST_TEST_CASE(&join_test));
    test->add(BOOST_TEST_CASE(&path_test));
    return test;
}
