// environment_test.cpp: test case for environment variables

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#include <hamigaki/process/environment.hpp>
#include <boost/test/unit_test.hpp>
#include <cstring>

namespace proc = hamigaki::process;
namespace ut = boost::unit_test;

void clear_test()
{
    proc::environment env;
    env.clear();

    char* s = env.data();
    BOOST_REQUIRE(s);
    BOOST_CHECK(*s == 0);
}

void set_test()
{
    proc::environment env;
    env.set("SET_TEST", "test");

    const char* s = env.get("SET_TEST");
    BOOST_REQUIRE(s);
    BOOST_CHECK(std::strcmp(s, "test") == 0);

    env.unset("SET_TEST");
    BOOST_CHECK(!env.get("SET_TEST"));
}

void data_test()
{
    proc::environment env;
    env.clear();
    env.set("DATA_TEST1", "foo");
    env.set("DATA_TEST2", "bar");

    char* s = env.data();
    BOOST_REQUIRE(s);
    BOOST_CHECK(std::memcmp(s, "DATA_TEST1=foo\0DATA_TEST2=bar\0", 31) == 0);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("environment variables test");
    test->add(BOOST_TEST_CASE(&clear_test));
    test->add(BOOST_TEST_CASE(&set_test));
    test->add(BOOST_TEST_CASE(&data_test));
    return test;
}
