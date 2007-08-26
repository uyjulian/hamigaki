// coro_copy_test.cpp: test case for the copy-ability of coroutine

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#include <hamigaki/coroutine/coroutine.hpp>
#include <boost/test/unit_test.hpp>

namespace coro = hamigaki::coroutines;
namespace ut = boost::unit_test;

typedef coro::coroutine<void(void)> coroutine_type0;
typedef coro::shared_coroutine<void(void)> shared_type0;
typedef coro::coroutine<void(void)> coroutine_type;
typedef coro::shared_coroutine<void(void)> shared_type;

void dummy_body(coroutine_type::self& self) {}

template<class Coro, class Shared>
void coro_copy_test_aux()
{
    Coro c1(dummy_body);
    BOOST_CHECK(!c1.empty());

    Coro c2(c1);
    BOOST_CHECK(c1.empty());
    BOOST_CHECK(!c2.empty());

    Coro c3;
    BOOST_CHECK(c3.empty());
    c3 = c2;
    BOOST_CHECK(c2.empty());
    BOOST_CHECK(!c3.empty());

    Coro s1(dummy_body);
    BOOST_CHECK(!s1.empty());

    Shared s2(s1);
    BOOST_CHECK(s1.empty());
    BOOST_CHECK(!s2.empty());

    Shared s3;
    BOOST_CHECK(s3.empty());
    s3 = s2;
    BOOST_CHECK(!s2.empty());
    BOOST_CHECK(!s3.empty());

    Shared s4;
    BOOST_CHECK(s4.empty());
    s4 = c3;
    BOOST_CHECK(c3.empty());
    BOOST_CHECK(!s4.empty());
}

void coro_copy_test()
{
    coro_copy_test_aux<coroutine_type0,shared_type0>();
    coro_copy_test_aux<coroutine_type0,shared_type>();
    coro_copy_test_aux<coroutine_type,shared_type0>();
    coro_copy_test_aux<coroutine_type,shared_type>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("coroutine copy test");
    test->add(BOOST_TEST_CASE(&coro_copy_test));
    return test;
}
