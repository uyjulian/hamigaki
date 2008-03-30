// expression_test.cpp: test case for bjam_expression_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/grammars/bjam_expression_grammar_gen.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam;
namespace ut = boost::unit_test;

bjam::string_list eval(bjam::context& ctx, const std::string& expr)
{
    typedef bjam::bjam_expression_grammar_gen<const char*> grammar_type;

    const char* first = expr.c_str();
    const char* last = first + expr.size();

    return grammar_type::evaluate(first, last, ctx);
}

void simple_test()
{
    bjam::context ctx;
    bjam::module& m = ctx.current_frame().current_module();
    bjam::variable_table& vars = m.variables;
    vars.set_values("X", boost::assign::list_of("a"));

    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "0");
    expect = boost::assign::list_of("0");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "$(X)");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "$()");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void eq_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "a = a");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "a = b");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void not_eq_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "a != a");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "a != b");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void lt_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "a < b");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "a < a");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "b < a");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void lt_eq_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "a <= b");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "a <= a");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "b <= a");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void gt_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "a > b");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "a > a");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "b > a");
    expect = boost::assign::list_of("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void gt_eq_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "a >= b");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "a >= a");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "b >= a");
    expect = boost::assign::list_of("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void and_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "a && b");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "$() && b");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "a && $()");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "$() && $()");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void or_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "a || b");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "$() || b");
    expect = boost::assign::list_of("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "a || $()");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "$() || $()");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void in_test()
{
    bjam::context ctx;
    bjam::module& m = ctx.current_frame().current_module();
    bjam::variable_table& vars = m.variables;
    vars.set_values("X", boost::assign::list_of("a")("b"));

    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "a in a");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "a in b");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "$(X) in b a");
    expect = boost::assign::list_of("a")("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "$(X) in a");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void not_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "! a");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "! $()");
    expect = boost::assign::list_of("1");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void paren_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "( a )");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "( $() )");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void short_circuit_test()
{
    bjam::context ctx;

    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "$() && [ EXIT ]");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "1 || [ EXIT ]");
    expect = boost::assign::list_of("1");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "$() in [ EXIT ]");
    expect = boost::assign::list_of("1");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("bjam_expression_grammar test");
    test->add(BOOST_TEST_CASE(&simple_test));
    test->add(BOOST_TEST_CASE(&eq_test));
    test->add(BOOST_TEST_CASE(&not_eq_test));
    test->add(BOOST_TEST_CASE(&lt_test));
    test->add(BOOST_TEST_CASE(&lt_eq_test));
    test->add(BOOST_TEST_CASE(&gt_test));
    test->add(BOOST_TEST_CASE(&gt_eq_test));
    test->add(BOOST_TEST_CASE(&and_test));
    test->add(BOOST_TEST_CASE(&or_test));
    test->add(BOOST_TEST_CASE(&in_test));
    test->add(BOOST_TEST_CASE(&not_test));
    test->add(BOOST_TEST_CASE(&paren_test));
    test->add(BOOST_TEST_CASE(&short_circuit_test));
    return test;
}
