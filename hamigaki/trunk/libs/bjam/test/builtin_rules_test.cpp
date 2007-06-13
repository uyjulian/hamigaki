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

void glob_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("."));
    args.push_back(boost::assign::list_of("*.cpp"));
    BOOST_CHECK(!ctx.invoke_rule("GLOB", args).empty());
}

void glob_recursive_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("./*.cpp"));
    BOOST_CHECK(!ctx.invoke_rule("GLOB-RECURSIVELY", args).empty());
}

void match_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("abc")("123")("def")("");

    args.push_back(boost::assign::list_of("^([a-z]+)([0-9]*)$"));
    args.push_back(boost::assign::list_of("abc123")("def"));
    result = ctx.invoke_rule("MATCH", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
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
            ("EXPORT")
            ("GLOB")
            ("GLOB-RECURSIVELY")
            ("IMPORT")
            ("IMPORT_MODULE")
            ("INSTANCE")
            ("MATCH")
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

    // clear pre-defined variables
    ctx.get_module(boost::none).variables.clear();

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

void import_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    BOOST_CHECK(ctx.invoke_rule("IMPORT", args).empty());

    args.push_back(bjam::string_list());
    args.push_back(boost::assign::list_of("EXIT"));
    args.push_back(boost::assign::list_of("m1"));
    args.push_back(boost::assign::list_of("ex"));
    BOOST_CHECK(ctx.invoke_rule("IMPORT", args).empty());
    {
        bjam::scoped_change_module guard(ctx, std::string("m1"));
        bjam::list_of_list args;
        BOOST_CHECK_THROW(ctx.invoke_rule("ex", args), bjam::exit_exception);
    }
}

void export_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list rules;

    BOOST_CHECK(ctx.invoke_rule("EXPORT", args).empty());


    rules = ctx.invoke_rule("RULENAMES", args);
    BOOST_CHECK(std::find(rules.begin(), rules.end(), "echo") == rules.end());

    args.push_back(bjam::string_list());
    args.push_back(boost::assign::list_of("echo"));
    BOOST_CHECK(ctx.invoke_rule("EXPORT", args).empty());

    rules = ctx.invoke_rule("RULENAMES", args);
    BOOST_CHECK(std::find(rules.begin(), rules.end(), "echo") != rules.end());
}

void import_module_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("m1")("m2"));
    args.push_back(boost::assign::list_of("m3"));
    BOOST_CHECK(ctx.invoke_rule("IMPORT_MODULE", args).empty());

    std::set<std::string> expect = boost::assign::list_of("m1")("m2");
    const std::set<std::string>& result =
        ctx.get_module(std::string("m3")).imported_modules;

    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void instance_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("obj"));
    args.push_back(boost::assign::list_of("cls"));
    BOOST_CHECK(ctx.invoke_rule("INSTANCE", args).empty());

    boost::optional<std::string> expect(std::string("cls"));
    BOOST_CHECK_EQUAL(ctx.get_module(std::string("obj")).class_module, expect);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("builtin rules test");
    test->add(BOOST_TEST_CASE(&echo_test));
    test->add(BOOST_TEST_CASE(&exit_test));
    test->add(BOOST_TEST_CASE(&glob_test));
    test->add(BOOST_TEST_CASE(&glob_recursive_test));
    test->add(BOOST_TEST_CASE(&match_test));
    test->add(BOOST_TEST_CASE(&rulenames_test));
    test->add(BOOST_TEST_CASE(&varnames_test));
    test->add(BOOST_TEST_CASE(&import_test));
    test->add(BOOST_TEST_CASE(&export_test));
    test->add(BOOST_TEST_CASE(&import_module_test));
    test->add(BOOST_TEST_CASE(&instance_test));
    return test;
}
