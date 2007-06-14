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

void always_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("ALWAYS", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::force_update);
}

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

void leaves_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("LEAVES", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::leaves);
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

void no_care_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("NOCARE", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::no_care);
}

void not_file_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("NOTFILE", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::not_file);
}

void no_update_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("NOUPDATE", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::no_update);
}

void temporary_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("TEMPORARY", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::temporary);
}

void is_file_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("ISFILE", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::is_file);
}

void fail_expected_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("FAIL_EXPECTED", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::fail_expected);
}

void rm_old_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("RMOLD", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::rm_old);
}

void rule_names_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;
    bjam::string_list expect;

    expect =
        boost::assign::list_of
            ("ALWAYS")
            ("CALC")
            ("ECHO")
            ("EXIT")
            ("EXPORT")
            ("FAIL_EXPECTED")
            ("GLOB")
            ("GLOB-RECURSIVELY")
            ("IMPORT")
            ("IMPORT_MODULE")
            ("INSTANCE")
            ("ISFILE")
            ("LEAVES")
            ("MATCH")
            ("NOCARE")
            ("NORMALIZE_PATH")
            ("NOTFILE")
            ("NOUPDATE")
            ("PWD")
            ("RMOLD")
            ("RULENAMES")
            ("SORT")
            ("TEMPORARY")
            ("VARNAMES")
        ;
    result = ctx.invoke_rule("RULENAMES", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void var_names_test()
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

void pwd_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;

    result = ctx.invoke_rule("PWD", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], ctx.working_directory());
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

void sort_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("car")("cat")("dog")("fox");

    args.push_back(boost::assign::list_of("dog")("cat")("fox")("car"));
    result = ctx.invoke_rule("SORT", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void normalize_path_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;

    result = ctx.invoke_rule("NORMALIZE_PATH", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], ".");
}

void calc_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;

    args.push_back(boost::assign::list_of("1")("+")("2"));
    result = ctx.invoke_rule("CALC", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], "3");

    args.clear();
    args.push_back(boost::assign::list_of("10")("-")("3"));
    result = ctx.invoke_rule("CALC", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], "7");
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("builtin rules test");
    test->add(BOOST_TEST_CASE(&always_test));
    test->add(BOOST_TEST_CASE(&echo_test));
    test->add(BOOST_TEST_CASE(&exit_test));
    test->add(BOOST_TEST_CASE(&glob_test));
    test->add(BOOST_TEST_CASE(&glob_recursive_test));
    test->add(BOOST_TEST_CASE(&leaves_test));
    test->add(BOOST_TEST_CASE(&match_test));
    test->add(BOOST_TEST_CASE(&no_care_test));
    test->add(BOOST_TEST_CASE(&not_file_test));
    test->add(BOOST_TEST_CASE(&no_update_test));
    test->add(BOOST_TEST_CASE(&temporary_test));
    test->add(BOOST_TEST_CASE(&is_file_test));
    test->add(BOOST_TEST_CASE(&fail_expected_test));
    test->add(BOOST_TEST_CASE(&rm_old_test));
    test->add(BOOST_TEST_CASE(&rule_names_test));
    test->add(BOOST_TEST_CASE(&var_names_test));
    test->add(BOOST_TEST_CASE(&import_test));
    test->add(BOOST_TEST_CASE(&export_test));
    test->add(BOOST_TEST_CASE(&pwd_test));
    test->add(BOOST_TEST_CASE(&import_module_test));
    test->add(BOOST_TEST_CASE(&instance_test));
    test->add(BOOST_TEST_CASE(&sort_test));
    test->add(BOOST_TEST_CASE(&normalize_path_test));
    test->add(BOOST_TEST_CASE(&calc_test));
    return test;
}
