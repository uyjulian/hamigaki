// bjam_test.cpp: test case for bjam_grammar

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam;
namespace algo = boost::algorithm;
namespace fs = boost::filesystem;
namespace ut = boost::unit_test;

std::string hamigaki_root;

bjam::string_list eval(bjam::context& ctx, const std::string& src)
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
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void set_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;
    bjam::string_list values;

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

void set_on_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;
    bjam::string_list values;

    result = eval(ctx, "A on t1 = ;");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "A on t1 = a ;");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("A");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());

    values = ctx.get_target("t1").variables.get_values("A");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());
}

void local_set_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;
    bjam::string_list values;

    result = eval(ctx, "local A = a ;");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("A");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());


    result = eval(ctx, "A = 1 ; local A = 2 ; B = $(A) ;");
    expect = boost::assign::list_of("2");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("A");
    expect = boost::assign::list_of("1");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("B");
    expect = boost::assign::list_of("2");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());
}

void return_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

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
    bjam::string_list result;
    bjam::string_list expect;

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

void for_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;
    bjam::string_list values;

    result = eval(ctx, "for A in { }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "for A in a b { }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("A");
    expect = boost::assign::list_of("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());


    result = eval(ctx, "for A in a b { B += $(A) ; }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("B");
    expect = boost::assign::list_of("a")("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());


    result = eval(ctx, "for local C in a b { }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("C");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());
}

void while_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;
    bjam::string_list values;

    result = eval(ctx, "while $() { }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "B = a a ; while $(A) != $(B) { A += a ; }");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());

    values = ctx.current_frame().current_module().variables.get_values("A");
    expect = boost::assign::list_of("a")("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        values.begin(), values.end(), expect.begin(), expect.end());
}

void switch_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "switch $() { }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "switch a { case a : return b ; }");
    expect = boost::assign::list_of("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "switch ab { case a? : return c ; }");
    expect = boost::assign::list_of("c");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "switch $() { case \"\" : return a ; }");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "switch ab { case cd : return e ; }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(
        ctx,
        "switch b { case a : EXIT ; case b : return 2 ; case c : EXIT ; }"
    );
    expect = boost::assign::list_of("2");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "switch a { case a : return 1 ; case a : return 2 ; }");
    expect = boost::assign::list_of("1");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void rule_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "rule r1 { }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "r1 ;");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "rule r2 { return a ; } r2 ;");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "rule r3 { return $(1) ; } r3 a b ;");
    expect = boost::assign::list_of("a")("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "rule r4 ( A ) { return $(A) ; } r4 a ;");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void module_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx, "module m1 { }");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "module m1 { A = a ; }");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "A = b ; module m1 { return $(A) ; }");
    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void class_test()
{
    bjam::context ctx;

    BOOST_CHECK(eval(ctx, "class c1 { }").empty());
    BOOST_CHECK(eval(ctx, "class c2 : c1 { }").empty());
    BOOST_CHECK(eval(ctx, "class c3 { rule r { } }").empty());
    BOOST_CHECK(eval(ctx, "class c4 : c3 { }").empty());

    bjam::module& m = ctx.get_module(std::string("class@c4"));
    m.rules.get_rule_definition("c3.r");
}

void on_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;
    bjam::string_list values;

    result = eval(ctx, "on t1 return ;");
    expect.clear();
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "A = 1 ; A on t1 = 2 ; on t1 return $(A) ;");
    expect = boost::assign::list_of("2");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void func_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    eval(ctx, "rule r1 return $(A) ;");
    eval(ctx, "A = 1 ; A on t1 = 2 ;");


    result = eval(ctx, "return [ r1 ] ;");
    expect = boost::assign::list_of("1");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "return [ on t1 r1 ] ;");
    expect = boost::assign::list_of("2");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    result = eval(ctx, "return [ on t1 return $(A) ] ;");
    expect = boost::assign::list_of("2");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

void include_test()
{
    bjam::context ctx;

    fs::path ph(hamigaki_root);
    ph /= "libs/bjam/test/include_test.jam";

    std::string src;
    src += "include ";
    src += algo::replace_all_copy(ph.file_string(), "\\", "\\\\");
    src += " ;";

    BOOST_CHECK(eval(ctx, src).empty());
}

void back_trace_test()
{
    bjam::context ctx;

    fs::path ph(hamigaki_root);
    ph /= "libs/bjam/test/back_trace_test.jam";
    std::string filename = ph.file_string();

    std::string src;
    src += "include ";
    src += algo::replace_all_copy(filename, "\\", "\\\\");
    src += " ;";

    BOOST_CHECK(eval(ctx, src).empty());

    bjam::module& m = ctx.get_module(std::string("bt_test"));
    const bjam::string_list& result = m.variables.get_values("result");

    bjam::string_list expect = boost::assign::list_of
        (filename)("9")("bt_test.")("back_trace")
        (filename)("17")("")("module scope")
        ;

    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());
}

ut::test_suite* init_unit_test_suite(int argc, char* argv[])
{
    if (argc != 2)
        return 0;

    hamigaki_root = argv[1];

    ut::test_suite* test = BOOST_TEST_SUITE("bjam_grammar test");
    test->add(BOOST_TEST_CASE(&empty_test));
    test->add(BOOST_TEST_CASE(&set_test));
    test->add(BOOST_TEST_CASE(&set_on_test));
    test->add(BOOST_TEST_CASE(&local_set_test));
    test->add(BOOST_TEST_CASE(&return_test));
    test->add(BOOST_TEST_CASE(&if_test));
    test->add(BOOST_TEST_CASE(&for_test));
    test->add(BOOST_TEST_CASE(&while_test));
    test->add(BOOST_TEST_CASE(&switch_test));
    test->add(BOOST_TEST_CASE(&rule_test));
    test->add(BOOST_TEST_CASE(&module_test));
    test->add(BOOST_TEST_CASE(&class_test));
    test->add(BOOST_TEST_CASE(&on_test));
    test->add(BOOST_TEST_CASE(&func_test));
    test->add(BOOST_TEST_CASE(&include_test));
    test->add(BOOST_TEST_CASE(&back_trace_test));
    return test;
}
