// exception_test.cpp: test case for exception handling about coroutines

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#include <hamigaki/coroutine/coroutine.hpp>
#include <boost/test/unit_test.hpp>
#include <stdexcept>

namespace coro = hamigaki::coroutines;
namespace ut = boost::unit_test;

typedef coro::coroutine<void(void)> coroutine_type;


void empty_func(coroutine_type::self& self)
{
}

void exit_func(coroutine_type::self& self)
{
    self.exit();
}

void exited_test()
{
    {
        coroutine_type c(empty_func);
        BOOST_REQUIRE_NO_THROW(c());
        BOOST_CHECK_THROW(c(), coro::coroutine_exited);
    }

    {
        coroutine_type c(exit_func);
        BOOST_CHECK_THROW(c(), coro::coroutine_exited);
    }
}


void throw_runtime_error(coroutine_type::self& self)
{
    throw std::runtime_error("throw_runtime_error()");
}

void throw_c_str(coroutine_type::self& self)
{
    throw "throw_c_str()";
}

bool is_runtime_error(const coro::abnormal_exit& e)
{
    return e.type() == typeid(std::runtime_error);
}

bool is_unknown_exception(const coro::abnormal_exit& e)
{
    return e.type() == typeid(coro::unknown_exception_tag);
}

void abnormal_exit_test()
{
    {
        coroutine_type c(throw_runtime_error);
        BOOST_CHECK_EXCEPTION(c(), coro::abnormal_exit, is_runtime_error);
    }

    {
        coroutine_type c(throw_c_str);
        BOOST_CHECK_EXCEPTION(c(), coro::abnormal_exit, is_unknown_exception);
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("exception test");
    test->add(BOOST_TEST_CASE(&exited_test));
    test->add(BOOST_TEST_CASE(&abnormal_exit_test));
    return test;
}
