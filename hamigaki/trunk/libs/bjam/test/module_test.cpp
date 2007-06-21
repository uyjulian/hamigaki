// module_test.cpp: test case for bjam module

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

void import_module_test()
{
    bjam::context ctx;
    bjam::string_list result;
    bjam::string_list expect;

    result = eval(ctx,
        "module m1 { "
            "rule r1 { "
                "return a ; "
            "} "
        "} "
        "IMPORT_MODULE m1 : m2 ; "
        "module m2 { "
            "m1.r1 ; "
        "}"
    );

    expect = boost::assign::list_of("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(
        result.begin(), result.end(), expect.begin(), expect.end());


    BOOST_CHECK_THROW(
        eval(ctx,
            "module m1 { "
                "local rule r2 { "
                    "return b ; "
                "} "
            "} "
            "module m2 { "
                "m1.r2 ; "
            "}"
        ),
        bjam::rule_not_found
    );
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("bjam module test");
    test->add(BOOST_TEST_CASE(&import_module_test));
    return test;
}
