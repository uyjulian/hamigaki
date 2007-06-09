// builtin_rules_test.cpp: test case for bjam builtin rules

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/bjam_context.hpp>
#include <hamigaki/bjam/bjam_exceptions.hpp>
#include <hamigaki/bjam/builtin_rules.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/none.hpp>
#include <cstdlib>
#include <iostream>

namespace bjam = hamigaki::bjam;
namespace ut = boost::unit_test;

bjam::string_list capture_echo(
    bjam::context& ctx, const bjam::list_of_list& args, std::stringbuf& buf)
{
    boost::io::ios_rdbuf_saver save(std::cout);

    buf.str(std::string());
    std::cout.rdbuf(&buf);

    return ctx.invoke_rule("ECHO", args);
}

void echo_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    std::stringbuf buf;

    BOOST_CHECK(capture_echo(ctx, args, buf).empty());
    BOOST_CHECK_EQUAL(buf.str(), std::string("\n"));

    args.push_back(boost::assign::list_of("a"));
    BOOST_CHECK(capture_echo(ctx, args, buf).empty());
    BOOST_CHECK_EQUAL(buf.str(), std::string("a\n"));

    args.clear();
    args.push_back(boost::assign::list_of("a")("b"));
    BOOST_CHECK(capture_echo(ctx, args, buf).empty());
    BOOST_CHECK_EQUAL(buf.str(), std::string("a b\n"));
}


class exit_checker
{
public:
    exit_checker(const char* msg, int code)
        : msg_(msg), code_(code)
    {
    }

    bool operator()(const bjam::exit_exception& e) const
    {
        return (e.what() == msg_) && (e.code() == code_);
    }

private:
    std::string msg_;
    int code_;
};

void exit_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    BOOST_CHECK_EXCEPTION(
        ctx.invoke_rule("EXIT", args), bjam::exit_exception,
        exit_checker("", EXIT_FAILURE)
    );

    args.push_back(boost::assign::list_of("a"));
    BOOST_CHECK_EXCEPTION(
        ctx.invoke_rule("EXIT", args), bjam::exit_exception,
        exit_checker("a", EXIT_FAILURE)
    );

    args.clear();
    args.push_back(boost::assign::list_of("a")("b"));
    BOOST_CHECK_EXCEPTION(
        ctx.invoke_rule("EXIT", args), bjam::exit_exception,
        exit_checker("a b", EXIT_FAILURE)
    );

    args.push_back(boost::assign::list_of("2"));
    BOOST_CHECK_EXCEPTION(
        ctx.invoke_rule("EXIT", args), bjam::exit_exception,
        exit_checker("a b", 2)
    );
}

void rulenames_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;
    bjam::string_list expect;

    expect =
        boost::assign::list_of
            ("ECHO")
            ("EXIT")
            ("RULENAMES")
            ("VARNAMES")
        ;
    result = ctx.invoke_rule("RULENAMES", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void varnames_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;
    bjam::string_list expect;

    result = ctx.invoke_rule("VARNAMES", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    bjam::variable_table& table = ctx.get_module(boost::none).variables;
    table.set_values("A", boost::assign::list_of("a"));
    expect.push_back("A");
    result = ctx.invoke_rule("VARNAMES", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("builtin rules test");
    test->add(BOOST_TEST_CASE(&echo_test));
    test->add(BOOST_TEST_CASE(&exit_test));
    test->add(BOOST_TEST_CASE(&rulenames_test));
    test->add(BOOST_TEST_CASE(&varnames_test));
    return test;
}
