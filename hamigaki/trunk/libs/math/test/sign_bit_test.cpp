// sign_bit_test.cpp: test case for sign_bit

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/math for library home page.

#include <hamigaki/math/sign_bit.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>
#include <typeinfo>

#if defined(__BORLANDC__)
    #include <float.h>
    #pragma option -w-8008 -w-8066
#endif

#if defined(_M_IX86) && \
    BOOST_WORKAROUND(__BORLANDC__, >= 0x580) && \
    BOOST_WORKAROUND(__BORLANDC__, <= 0x582)
std::_Dconst _FInf  = {{ 0x0000, 0x7F80 }};
std::_Dconst _FNan  = {{ 0x0000, 0x7FC0 }};
std::_Dconst _FSnan = {{ 0x0001, 0x7F80 }};

std::_Dconst _Inf   = {{ 0x0000, 0x0000, 0x0000, 0x7FF0 }};
std::_Dconst _Nan   = {{ 0x0000, 0x0000, 0x0000, 0x7FF8 }};
std::_Dconst _Snan  = {{ 0x0001, 0x0000, 0x0000, 0x7FF0 }};

std::_Dconst _LInf  = {{ 0x0000, 0x0000, 0x0000, 0x8000, 0x7FFF }};
std::_Dconst _LNan  = {{ 0x0000, 0x0000, 0x0000, 0xC000, 0x7FFF }};
std::_Dconst _LSnan = {{ 0x0001, 0x0000, 0x0000, 0x8000, 0x7FFF }};
#endif

namespace math_ex = hamigaki::math;
namespace ut = boost::unit_test;

#if defined(_MSC_VER) && defined(_M_AMD64)
float flip_to_minus(float x)
{
    return static_cast<float>(_copysign(static_cast<double>(x), -1.0));
}

double flip_to_minus(double x)
{
    return _copysign(x, -1.0);
}

long double flip_to_minus(long double x)
{
    return _copysignl(x, -1.0l);
}

int flip_to_minus(int x)
{
    return -x;
}
#else
template<typename T>
T flip_to_minus(T x)
{
    return -x;
}
#endif

template<typename T>
void sign_bit_test_aux()
{
    std::cout << "Testing for " << typeid(T).name() << std::endl;

    typedef std::numeric_limits<T> traits;

    BOOST_CHECK(!math_ex::sign_bit(T()));

    if (traits::is_specialized)
    {
        BOOST_CHECK(!math_ex::sign_bit((traits::max)()));
        BOOST_CHECK(math_ex::sign_bit(flip_to_minus((traits::max)())));
    }

    if (traits::has_infinity)
    {
        BOOST_CHECK(!math_ex::sign_bit(traits::infinity()));
        BOOST_CHECK(math_ex::sign_bit(flip_to_minus(traits::infinity())));
    }

    if (traits::has_quiet_NaN)
    {
        BOOST_CHECK(!math_ex::sign_bit(traits::quiet_NaN()));
        BOOST_CHECK(math_ex::sign_bit(flip_to_minus(traits::quiet_NaN())));
    }

    if (traits::has_signaling_NaN)
    {
        BOOST_CHECK(!math_ex::sign_bit(traits::signaling_NaN()));
        BOOST_CHECK(math_ex::sign_bit(flip_to_minus(traits::signaling_NaN())));
    }
}

template<typename T>
void sign_bit_test_non_number()
{
    std::cout << "Testing for " << typeid(T).name() << std::endl;

    BOOST_CHECK(!math_ex::sign_bit(T()));
}

void sign_bit_test()
{
#if defined(__BORLANDC__)
     ::_control87(MCW_EM,MCW_EM);
#endif

    sign_bit_test_aux<float>();
    sign_bit_test_aux<double>();
    sign_bit_test_aux<long double>();

    sign_bit_test_aux<int>();

    sign_bit_test_non_number<std::string>();

#if defined(__BORLANDC__)
     ::_clear87();
#endif
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("sign_bit test");
    test->add(BOOST_TEST_CASE(&sign_bit_test));
    return test;
}
