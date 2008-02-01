// transport_test.cpp: test case for N2179

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <hamigaki/exception.hpp>
#include <boost/test/unit_test.hpp>

namespace ut = boost::unit_test;

struct my_exception{};

void rethrow_test()
{
    hamigaki::exception_ptr p = hamigaki::copy_exception(my_exception());

    BOOST_CHECK_THROW(hamigaki::rethrow_exception(p), my_exception);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("exception transport test");
    test->add(BOOST_TEST_CASE(&rethrow_test));
    return test;
}
