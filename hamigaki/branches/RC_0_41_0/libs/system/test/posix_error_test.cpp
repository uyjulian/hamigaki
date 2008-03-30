// posix_error_test.cpp: test case for posix_error

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/uuid for library home page.

#include <hamigaki/system/posix_error.hpp>
#include <boost/test/unit_test.hpp>

namespace sys = hamigaki::system;
namespace ut = boost::unit_test;

typedef sys::posix_error_traits traits_type;

void msg_test()
{
    const std::string& msg1 = traits_type::message(EDOM);
    const std::string& msg2 = traits_type::message(EILSEQ);

    BOOST_CHECK(!msg1.empty());
    BOOST_CHECK(!msg2.empty());
    BOOST_CHECK(msg1 != msg2);
}

void error_test()
{
    sys::posix_error e(EDOM, "error_test()");
    BOOST_CHECK_EQUAL(e.code(), EDOM);

    std::string s(e.what());
    BOOST_CHECK_EQUAL(s.substr(0, 14), std::string("error_test(): "));
    BOOST_CHECK_EQUAL(s.substr(14), traits_type::message(EDOM));
}

void empty_test()
{
    sys::posix_error e;
    BOOST_CHECK_EQUAL(e.code(), 0);

    std::string s(e.what());
    BOOST_CHECK_EQUAL(s, traits_type::message(0));
}

void copy_test()
{
    sys::posix_error e(EDOM, "copy_test()");
    sys::posix_error e2(e);

    BOOST_CHECK_EQUAL(e.what(), e2.what());
}

void assign_test()
{
    sys::posix_error e(EDOM, "foo()");
    sys::posix_error e2(EILSEQ, "bar()");
    e2 = e;

    e2.what();

    BOOST_CHECK_EQUAL(e.what(), e2.what());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("posix_error test");
    test->add(BOOST_TEST_CASE(&msg_test));
    test->add(BOOST_TEST_CASE(&error_test));
    test->add(BOOST_TEST_CASE(&empty_test));
    test->add(BOOST_TEST_CASE(&copy_test));
    test->add(BOOST_TEST_CASE(&assign_test));
    return test;
}
