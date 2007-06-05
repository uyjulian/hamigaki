// bjam_test.cpp: test case for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam;
namespace ut = boost::unit_test;

bjam::list_type eval(bjam::context& ctx, const std::string& src)
{
    typedef bjam::bjam_grammar_gen<const char*> grammar_type;

    const char* first = src.c_str();
    const char* last = first + src.size();

    bjam::parse_info<const char*> info =
        grammar_type::parse_bjam_grammar(first, last, ctx);

    BOOST_CHECK(info.full);

    return info.values;
}

void empty_test()
{
    bjam::context ctx;
    bjam::list_type result;
    bjam::list_type expect;

    result = eval(ctx, "");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void set_test()
{
    bjam::context ctx;
    bjam::list_type result;
    bjam::list_type expect;
    bjam::list_type values;

    result = eval(ctx, "A = ;");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "A = a ;");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "A += b ;");
    expect = boost::assign::list_of("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("A");
    expect = boost::assign::list_of("a")("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());


    result = eval(ctx, "A ?= c ;");
    expect = boost::assign::list_of("c");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("A");
    expect = boost::assign::list_of("a")("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());


    result = eval(ctx, "C ?= c ;");
    expect = boost::assign::list_of("c");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("C");
    expect = boost::assign::list_of("c");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());
}

void return_test()
{
    bjam::context ctx;
    bjam::list_type result;
    bjam::list_type expect;

    result = eval(ctx, "return a ;");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "return ;");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void if_test()
{
    bjam::context ctx;
    bjam::list_type result;
    bjam::list_type expect;

    result = eval(ctx, "if a { return b ; }");
    expect = boost::assign::list_of("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "if $() { return b ; }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "if a { return b ; } else { return c ; }");
    expect = boost::assign::list_of("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    result = eval(ctx, "if $() { return b ; } else { return c ; }");
    expect = boost::assign::list_of("c");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("bjam_grammar test");
    test->add(BOOST_TEST_CASE(&empty_test));
    test->add(BOOST_TEST_CASE(&set_test));
    test->add(BOOST_TEST_CASE(&return_test));
    test->add(BOOST_TEST_CASE(&if_test));
    return test;
}
