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
#include <boost/range/empty.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/none.hpp>
#include <cstdlib>
#include <iostream>

namespace bjam = hamigaki::bjam;
namespace ut = boost::unit_test;

void always_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    BOOST_CHECK(ctx.invoke_rule("ALWAYS", args).empty());
    BOOST_CHECK(ctx.get_target("t1").flags & bjam::target::force_update);
}

void depends_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    args.push_back(boost::assign::list_of("t2"));
    BOOST_CHECK(ctx.invoke_rule("DEPENDS", args).empty());

    std::set<std::string>& table = ctx.get_target("t1").depended_targets;
    BOOST_CHECK(table.find("t2") != table.end());
}

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
    args.push_back(boost::assign::list_of("*.v2"));
    BOOST_CHECK(!ctx.invoke_rule("GLOB", args).empty());
}

void glob_recursive_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("./*.v2"));
    BOOST_CHECK(!ctx.invoke_rule("GLOB-RECURSIVELY", args).empty());
}

void includes_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    args.push_back(boost::assign::list_of("t2"));
    BOOST_CHECK(ctx.invoke_rule("INCLUDES", args).empty());

    std::set<std::string>& table = ctx.get_target("t1").included_targets;
    BOOST_CHECK(table.find("t2") != table.end());
}

void rebuilds_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    args.push_back(boost::assign::list_of("t1"));
    args.push_back(boost::assign::list_of("t2"));
    BOOST_CHECK(ctx.invoke_rule("REBUILDS", args).empty());

    std::set<std::string>& table = ctx.get_target("t1").rebuilt_targets;
    BOOST_CHECK(table.find("t2") != table.end());
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

void update_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("t1")("t2");
    args.push_back(expect);
    BOOST_CHECK(ctx.invoke_rule("UPDATE", args).empty());
    result = ctx.targets_to_update();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    args.clear();
    args.push_back(boost::assign::list_of("t3"));
    result = ctx.invoke_rule("UPDATE", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
    result = ctx.targets_to_update();
    expect = boost::assign::list_of("t3");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void subst_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of
        ("abc")
        ("abc-123")
        ("abc-123")
        ("123-abc")
    ;

    args.push_back(
        boost::assign::list_of
            ("abc123")
            ("^([a-z]+)([0-9]*)$")
            ("$1")
            ("$1-$2")
            ("\\1-\\2")
            ("\\2-$1")
    );
    result = ctx.invoke_rule("SUBST", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
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
            ("BACKTRACE")
            ("CALC")
            ("CALLER_MODULE")
            ("COMMAND")
            ("DELETE_MODULE")
            ("DEPENDS")
            ("ECHO")
            ("EXIT")
            ("EXPORT")
            ("FAIL_EXPECTED")
            ("GLOB")
            ("GLOB-RECURSIVELY")
            ("IMPORT")
            ("IMPORT_MODULE")
            ("INCLUDES")
            ("INSTANCE")
            ("ISFILE")
            ("LEAVES")
            ("MATCH")
            ("NOCARE")
            ("NORMALIZE_PATH")
            ("NOTFILE")
            ("NOUPDATE")
            ("PWD")
            ("REBUILDS")
            ("RMOLD")
            ("RULENAMES")
            ("SHELL")
            ("SORT")
            ("SUBST")
            ("TEMPORARY")
            ("UPDATE")
            ("VARNAMES")
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
            ("W32_GETREG")
#endif
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

void delete_module_test()
{
    bjam::context ctx;
    bjam::list_of_list args;

    BOOST_CHECK(ctx.invoke_rule("DELETE_MODULE", args).empty());

    bjam::module& m = ctx.get_module(boost::none);
    BOOST_CHECK(boost::empty(m.rules.entries()));
    BOOST_CHECK(boost::empty(m.variables.entries()));
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

void caller_module_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;

    BOOST_CHECK(ctx.invoke_rule("CALLER_MODULE", args).empty());


    bjam::module& m1 = ctx.get_module(std::string("m1"));
    bjam::module& m2 = ctx.get_module(std::string("m2"));
    bjam::module& m3 = ctx.get_module(std::string("m3"));

    ctx.push_frame(bjam::frame(m1, std::string("m1")));
    ctx.push_frame(bjam::frame(m2, std::string("m2")));
    ctx.push_frame(bjam::frame(m3, std::string("m3")));


    result = ctx.invoke_rule("CALLER_MODULE", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], "m2");

    args.push_back(boost::assign::list_of("1"));
    result = ctx.invoke_rule("CALLER_MODULE", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], "m1");

    args.clear();
    args.push_back(boost::assign::list_of("-1"));
    result = ctx.invoke_rule("CALLER_MODULE", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], "m3");
}

void back_trace_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;
    bjam::string_list expect;

    expect = boost::assign::list_of("")("1")("")("module scope");
    result = ctx.invoke_rule("BACKTRACE", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    bjam::module& m1 = ctx.get_module(std::string("m1"));
    bjam::frame f1(m1, std::string("m1"));
    f1.filename("f1.jam");
    f1.line(123);
    f1.rule_name(std::string("r1"));
    ctx.push_frame(f1);

    expect = boost::assign::list_of
        ("f1.jam")("123")("m1.")("r1")
        ("")("1")("")("module scope")
        ;
    result = ctx.invoke_rule("BACKTRACE", args);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
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

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
#include <windows.h>
void w32_getreg_test()
{
    char win_dir[MAX_PATH];
    ::GetWindowsDirectoryA(win_dir, sizeof(win_dir));

    // should use SHGetSpecialFolderPath()
    std::string prog_dir(win_dir, 0, 3);
    prog_dir += "Program Files";

    ::OSVERSIONINFOA info;
    std::memset(&info, 0, sizeof(info));
    info.dwOSVersionInfoSize = sizeof(info);
    ::GetVersionExA(&info);
    int build_num = static_cast<int>(info.dwBuildNumber);


    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;

    args.push_back(boost::assign::list_of
        ("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
        ("ProgramFilesDir")
    );
    result = ctx.invoke_rule("W32_GETREG", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], prog_dir);


    // Note:
    // Cygwin remove many environment strings.
    // So REG_EXPAND_SZ does not work in many cases.
#if !defined(__CYGWIN__)
    args.clear();
    args.push_back(boost::assign::list_of
        ("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
        ("ProgramFilesPath")
    );
    result = ctx.invoke_rule("W32_GETREG", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], prog_dir);
#endif


    args.clear();
    args.push_back(boost::assign::list_of
        ("HKEY_CLASSES_ROOT\\.exe")
    );
    result = ctx.invoke_rule("W32_GETREG", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(result[0], "exefile");


    args.clear();
    args.push_back(boost::assign::list_of
        ("HKCU\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
        ("BuildNumber")
    );
    result = ctx.invoke_rule("W32_GETREG", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
        BOOST_CHECK_EQUAL(std::atoi(result[0].c_str()), build_num);


    args.clear();
    args.push_back(boost::assign::list_of
        ("HKLM\\SYSTEM\\CurrentControlSet\\Services\\PlugPlay")
        ("RequiredPrivileges")
    );
    result = ctx.invoke_rule("W32_GETREG", args);
    BOOST_CHECK(result.size() >= 2);
    BOOST_CHECK(
        std::find(
            result.begin(), result.end(),
            "SeLoadDriverPrivilege"
        ) != result.end()
    );
}
#endif

void shell_test()
{
    bjam::context ctx;
    bjam::list_of_list args;
    bjam::string_list result;

    args.push_back(boost::assign::list_of("echo hello"));
    result = ctx.invoke_rule("SHELL", args);
    BOOST_CHECK_EQUAL(result.size(), 1u);
    if (!result.empty())
    {
#if defined(BOOST_WINDOWS) && !defined(__CYGWIN__)
        BOOST_CHECK_EQUAL(result[0], "hello\r\n");
#else
        BOOST_CHECK_EQUAL(result[0], "hello\n");
#endif
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("builtin rules test");
    test->add(BOOST_TEST_CASE(&always_test));
    test->add(BOOST_TEST_CASE(&depends_test));
    test->add(BOOST_TEST_CASE(&echo_test));
    test->add(BOOST_TEST_CASE(&exit_test));
    test->add(BOOST_TEST_CASE(&glob_test));
    test->add(BOOST_TEST_CASE(&glob_recursive_test));
    test->add(BOOST_TEST_CASE(&includes_test));
    test->add(BOOST_TEST_CASE(&rebuilds_test));
    test->add(BOOST_TEST_CASE(&leaves_test));
    test->add(BOOST_TEST_CASE(&match_test));
    test->add(BOOST_TEST_CASE(&no_care_test));
    test->add(BOOST_TEST_CASE(&not_file_test));
    test->add(BOOST_TEST_CASE(&no_update_test));
    test->add(BOOST_TEST_CASE(&temporary_test));
    test->add(BOOST_TEST_CASE(&is_file_test));
    test->add(BOOST_TEST_CASE(&fail_expected_test));
    test->add(BOOST_TEST_CASE(&rm_old_test));
    test->add(BOOST_TEST_CASE(&update_test));
    test->add(BOOST_TEST_CASE(&subst_test));
    test->add(BOOST_TEST_CASE(&rule_names_test));
    test->add(BOOST_TEST_CASE(&var_names_test));
    test->add(BOOST_TEST_CASE(&delete_module_test));
    test->add(BOOST_TEST_CASE(&import_test));
    test->add(BOOST_TEST_CASE(&export_test));
    test->add(BOOST_TEST_CASE(&caller_module_test));
    test->add(BOOST_TEST_CASE(&back_trace_test));
    test->add(BOOST_TEST_CASE(&pwd_test));
    test->add(BOOST_TEST_CASE(&import_module_test));
    test->add(BOOST_TEST_CASE(&instance_test));
    test->add(BOOST_TEST_CASE(&sort_test));
    test->add(BOOST_TEST_CASE(&normalize_path_test));
    test->add(BOOST_TEST_CASE(&calc_test));
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    test->add(BOOST_TEST_CASE(&w32_getreg_test));
#endif
    test->add(BOOST_TEST_CASE(&shell_test));
    return test;
}
