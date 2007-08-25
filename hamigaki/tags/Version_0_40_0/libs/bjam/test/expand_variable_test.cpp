// expand_variable_test.cpp: test case for expand_variable()

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

    bjam::string_list result;
    bjam::string_list expect;

    result = bjam::expand_variable("", table, args);
    expect = boost::assign::list_of("");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("abc");
    result = bjam::expand_variable("abc", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("hoge");
    result = bjam::expand_variable("$(X)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void index_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("a")("b")("c"));
    bjam::list_of_list args;

    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("a");
    result = bjam::expand_variable("$(X[1])", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("b")("c");
    result = bjam::expand_variable("$(X[2-3])", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("b")("c");
    result = bjam::expand_variable("$(X[2-])", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("c");
    result = bjam::expand_variable("$(X[-1])", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void case_conv_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("aBcD"));
    bjam::list_of_list args;

    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("abcd");
    result = bjam::expand_variable("$(X:L)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("ABCD");
    result = bjam::expand_variable("$(X:U)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("ABCD");
    result = bjam::expand_variable("$(X:UL)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void empty_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("a")("b")("c"));
    bjam::list_of_list args;

    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("hoge");
    result = bjam::expand_variable("$(:E=hoge)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("a")("b")("c");
    result = bjam::expand_variable("$(:E=$(X))", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("a")("b")("c");
    result = bjam::expand_variable("$([1]:E=$(X))", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void join_test()
{
    bjam::variable_table table;
    table.set_values("X", boost::assign::list_of("a")("b")("c"));
    table.set_values("Y", boost::assign::list_of(".")("-"));

    bjam::list_of_list args;

    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("a_b_c");
    result = bjam::expand_variable("$(X:J=_)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    // Note: This may be a bjam bug
    result.clear();
    expect = boost::assign::list_of("a-b-c");
    result = bjam::expand_variable("$(X:J=$(Y))", table, args);
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

    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("b.s");
    result = bjam::expand_variable("$(X:BS)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("<g>/d/b.txt(m)");
    result = bjam::expand_variable("$(X:S=.txt)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("<g>/d");
    result = bjam::expand_variable("$(X:P)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("<g>/d/b.s(m)");
    result = bjam::expand_variable("$(X:R=$(TMP))", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("C:/Windows/System32");
    result = bjam::expand_variable("$(SYS:T)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("@<g>@");
    result = bjam::expand_variable("@$(:E=:G=g)@", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("@b@");
    result = bjam::expand_variable("@$(:E=:B=b)@", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
#if defined(__CYGWIN__)
    // FIXME: Cygwin may not be installed to the default location
    expect = boost::assign::list_of("C:\\cygwin\\tmp");
#else
    expect = boost::assign::list_of("/tmp");
#endif
    result = bjam::expand_variable("$(TMP:W)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

#if defined(BOOST_WINDOWS)
    table.set_values("A", boost::assign::list_of("C:\\A\\a.txt"));
    table.set_values("B", boost::assign::list_of("C:\\B\\dir"));

    result.clear();
    expect = boost::assign::list_of("C:\\A\\a.txt");
    result = bjam::expand_variable("$(A:R=$(B))", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
#endif
}

void args_test()
{
    bjam::variable_table table;

    bjam::list_of_list args;
    args.push_back(boost::assign::list_of("a")("b")("c"));
    args.push_back(boost::assign::list_of("1")("2"));

    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("a")("b")("c");
    result = bjam::expand_variable("$(1)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("1")("2");
    result = bjam::expand_variable("$(2)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("a")("b")("c");
    result = bjam::expand_variable("$(<)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("1")("2");
    result = bjam::expand_variable("$(>)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void fixed_test()
{
    bjam::variable_table table;
    bjam::list_of_list args;
    bjam::string_list result;
    bjam::string_list expect;

    result = bjam::expand_variable("$(TMPDIR)", table, args);
    BOOST_CHECK(!result.empty());

    result.clear();
    result = bjam::expand_variable("$(TMPNAME)", table, args);
    BOOST_CHECK(!result.empty());

    result.clear();
    result = bjam::expand_variable("$(TMPFILE)", table, args);
    BOOST_CHECK(!result.empty());

    result.clear();
    expect = boost::assign::list_of("STDOUT");
    result = bjam::expand_variable("$(STDOUT)", table, args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result.clear();
    expect = boost::assign::list_of("STDERR");
    result = bjam::expand_variable("$(STDERR)", table, args);
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
    test->add(BOOST_TEST_CASE(&args_test));
    test->add(BOOST_TEST_CASE(&fixed_test));
    return test;
}
