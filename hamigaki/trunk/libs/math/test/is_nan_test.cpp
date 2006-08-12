//  is_nan_test.cpp: test case for is_nan

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/math for library home page.

#include <hamigaki/math/is_nan.hpp>
#include <boost/test/unit_test.hpp>
#include <string>
#include <typeinfo>

namespace math_ex = hamigaki::math;
namespace ut = boost::unit_test;

template<typename T>
void is_nan_test_aux()
{
    typedef std::numeric_limits<T> traits;

    BOOST_CHECK_MESSAGE(!math_ex::is_nan(T()), typeid(T).name());

    if (traits::is_specialized)
    {
        BOOST_CHECK_MESSAGE(
            !math_ex::is_nan((traits::max)()),
            typeid(T).name()
        );
    }

    if (traits::has_infinity )
    {
        BOOST_CHECK_MESSAGE(
            !math_ex::is_nan(traits::infinity ()),
            typeid(T).name()
        );
    }

    if (traits::has_quiet_NaN)
    {
        BOOST_CHECK_MESSAGE(
            math_ex::is_nan(traits::quiet_NaN()),
            typeid(T).name()
        );
    }

    if (traits::has_signaling_NaN)
    {
        BOOST_CHECK_MESSAGE(
            math_ex::is_nan(traits::signaling_NaN()),
            typeid(T).name()
        );
    }
}

void is_nan_test()
{
    is_nan_test_aux<float>();
    is_nan_test_aux<double>();
    is_nan_test_aux<long double>();

    is_nan_test_aux<int>();
    is_nan_test_aux<std::string>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("is_nan test");
    test->add(BOOST_TEST_CASE(&is_nan_test));
    return test;
}
