// get_variable_values_test.cpp: test case for get_variable_values()

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

    {
        bjam::string_list expect = boost::assign::list_of("");
        bjam::string_list buf;
        const bjam::string_list& result =
            bjam::get_variable_values(buf, "", table);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            result.begin(), result.end(), expect.begin(), expect.end());
    }

    {
        bjam::string_list expect;
        bjam::string_list buf;
        const bjam::string_list& result =
            bjam::get_variable_values(buf, "abc", table);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            result.begin(), result.end(), expect.begin(), expect.end());
    }

    {
        bjam::string_list expect = boost::assign::list_of("hoge");
        bjam::string_list buf;
        const bjam::string_list& result =
            bjam::get_variable_values(buf, "X", table);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            result.begin(), result.end(), expect.begin(), expect.end());
    }
}

void args_test()
{
    bjam::variable_table table;

    bjam::list_of_list args;
    args.push_back(boost::assign::list_of("a")("b")("c"));
    args.push_back(boost::assign::list_of("1")("2"));

    {
        bjam::string_list expect = boost::assign::list_of("a")("b")("c");
        bjam::string_list buf;
        const bjam::string_list& result =
            bjam::get_variable_values(buf, "1", table, args);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            result.begin(), result.end(), expect.begin(), expect.end());
    }

    {
        bjam::string_list expect = boost::assign::list_of("1")("2");
        bjam::string_list buf;
        const bjam::string_list& result =
            bjam::get_variable_values(buf, "2", table, args);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            result.begin(), result.end(), expect.begin(), expect.end());
    }

    {
        bjam::string_list expect = boost::assign::list_of("a")("b")("c");
        bjam::string_list buf;
        const bjam::string_list& result =
            bjam::get_variable_values(buf, "<", table, args);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            result.begin(), result.end(), expect.begin(), expect.end());
    }

    {
        bjam::string_list expect = boost::assign::list_of("1")("2");
        bjam::string_list buf;
        const bjam::string_list& result =
            bjam::get_variable_values(buf, ">", table, args);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            result.begin(), result.end(), expect.begin(), expect.end());
    }
}

void fixed_test()
{
    bjam::variable_table table;
    bjam::string_list expect;

    {
        bjam::string_list buf;
        const bjam::string_list& result
            = bjam::get_variable_values(buf, "TMPDIR", table);
        BOOST_CHECK(&result == &buf);
    }

    {
        bjam::string_list buf;
        const bjam::string_list& result
            = bjam::get_variable_values(buf, "TMPNAME", table);
        BOOST_CHECK(&result == &buf);
    }

    {
        bjam::string_list buf;
        const bjam::string_list& result
            = bjam::get_variable_values(buf, "TMPFILE", table);
        BOOST_CHECK(&result == &buf);
    }

    expect = boost::assign::list_of("STDOUT");
    {
        bjam::string_list buf;
        const bjam::string_list& result
            = bjam::get_variable_values(buf, "STDOUT", table);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            result.begin(), result.end(), expect.begin(), expect.end());
    }

    expect = boost::assign::list_of("STDERR");
    {
        bjam::string_list buf;
        const bjam::string_list& result
            = bjam::get_variable_values(buf, "STDERR", table);
        BOOST_CHECK_EQUAL_COLLECTIONS(
            result.begin(), result.end(), expect.begin(), expect.end());
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("get_variable_values test");
    test->add(BOOST_TEST_CASE(&args_test));
    test->add(BOOST_TEST_CASE(&fixed_test));
    return test;
}
