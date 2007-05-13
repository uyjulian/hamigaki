// member_access_traits_test.cpp: test case for member_access_traits

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/type_traits for library home page.

#include <hamigaki/type_traits/member_access_traits.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/type_traits/is_same.hpp>

namespace ut = boost::unit_test;

template<class T, class U, class Reference, class Pointer>
void member_access_check()
{
    typedef hamigaki::member_access_traits<T,U> traits;

    BOOST_MPL_ASSERT((boost::is_same<typename traits::reference, Reference>));
    BOOST_MPL_ASSERT((boost::is_same<typename traits::pointer, Pointer>));
}

void member_access_traits_test()
{
    member_access_check<int, int, int&, int*>();

    member_access_check<const int, int, const int&, const int*>();
    member_access_check<const int, const int, const int&, const int*>();
    member_access_check<const int, int&, int&, int*>();

    member_access_check<volatile int, int, volatile int&, volatile int*>();
    member_access_check<
        volatile int, volatile int, volatile int&, volatile int*>();
    member_access_check<volatile int, int&, int&, int*>();

    member_access_check<
        const int, volatile int, const volatile int&, const volatile int*>();
    member_access_check<
        volatile int, const int, const volatile int&, const volatile int*>();
    member_access_check<
        const volatile int, int, const volatile int&, const volatile int*>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("member_access_traits test");
    test->add(BOOST_TEST_CASE(&member_access_traits_test));
    return test;
}
