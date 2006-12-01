//  auto_min_test.cpp: test case for auto_min

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/integer for library home page.

#include <hamigaki/integer/auto_min.hpp>
#include <boost/integer/static_min_max.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>
#include <boost/integer_traits.hpp>
#include <boost/static_assert.hpp>

namespace ut = boost::unit_test;
using hamigaki::detail::auto_min_result;

template<typename T, typename U>
struct auto_min_range_check
{
    typedef typename auto_min_result<T,U>::type result_type;

    static const boost::int64_t t_min =
        static_cast<boost::int64_t>(
            boost::integer_traits<T>::const_min);

    static const boost::int64_t t_max =
        static_cast<boost::int64_t>(
            boost::integer_traits<T>::const_max);

    static const boost::int64_t u_min =
        static_cast<boost::int64_t>(
            boost::integer_traits<U>::const_min);

    static const boost::int64_t u_max =
        static_cast<boost::int64_t>(
            boost::integer_traits<U>::const_max);

    static const boost::int64_t r_min =
        static_cast<boost::int64_t>(
            boost::integer_traits<result_type>::const_min);

    static const boost::int64_t r_max =
        static_cast<boost::int64_t>(
            boost::integer_traits<result_type>::const_max);

    static const bool value =
        (r_min <= t_min) &&
        (r_min <= u_min) &&
        (t_max < u_max ? t_max <= r_max : u_max <= r_max);
};

void auto_min_result_test()
{
    using namespace boost;

    BOOST_STATIC_ASSERT(( auto_min_range_check<int8_t  ,int8_t  >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int8_t  ,uint8_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int8_t  ,int16_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int8_t  ,uint16_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int8_t  ,int32_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int8_t  ,uint32_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int8_t  ,int64_t >::value ));

    BOOST_STATIC_ASSERT(( auto_min_range_check<uint8_t ,int8_t  >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint8_t ,uint8_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint8_t ,int16_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint8_t ,uint16_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint8_t ,int32_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint8_t ,uint32_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint8_t ,int64_t >::value ));

    BOOST_STATIC_ASSERT(( auto_min_range_check<int16_t ,int8_t  >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int16_t ,uint8_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int16_t ,int16_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int16_t ,uint16_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int16_t ,int32_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int16_t ,uint32_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int16_t ,int64_t >::value ));

    BOOST_STATIC_ASSERT(( auto_min_range_check<uint16_t,int8_t  >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint16_t,uint8_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint16_t,int16_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint16_t,uint16_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint16_t,int32_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint16_t,uint32_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint16_t,int64_t >::value ));

    BOOST_STATIC_ASSERT(( auto_min_range_check<int32_t ,int8_t  >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int32_t ,uint8_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int32_t ,int16_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int32_t ,uint16_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int32_t ,int32_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int32_t ,uint32_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int32_t ,int64_t >::value ));

    BOOST_STATIC_ASSERT(( auto_min_range_check<uint32_t,int8_t  >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint32_t,uint8_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint32_t,int16_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint32_t,uint16_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint32_t,int32_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint32_t,uint32_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<uint32_t,int64_t >::value ));

    BOOST_STATIC_ASSERT(( auto_min_range_check<int64_t ,int8_t  >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int64_t ,uint8_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int64_t ,int16_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int64_t ,uint16_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int64_t ,int32_t >::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int64_t ,uint32_t>::value ));
    BOOST_STATIC_ASSERT(( auto_min_range_check<int64_t ,int64_t >::value ));
}

void auto_min_test()
{
    using hamigaki::auto_min;
    using namespace boost;

    BOOST_CHECK_EQUAL(auto_min(0,0), 0);
    BOOST_CHECK_EQUAL(auto_min(1,2), 1);
    BOOST_CHECK_EQUAL(auto_min(1,0u), 0);
    BOOST_CHECK_EQUAL(auto_min(-1,1), -1);
    BOOST_CHECK_EQUAL(auto_min(1,-1), -1);
    BOOST_CHECK_EQUAL(auto_min(-2147483647,0xFFFFFFFFu), -2147483647);
    BOOST_CHECK_EQUAL(auto_min(-2147483647-1,0xFFFFFFFFu), -2147483647-1);
    BOOST_CHECK_EQUAL(auto_min(-1,0xFFFFFFFFu), -1);
    BOOST_CHECK_EQUAL(auto_min(-2,0xFFFFFFFFu), -2);
    BOOST_CHECK_EQUAL(auto_min(1,0u), 0);

    BOOST_CHECK_EQUAL(
        auto_min(
            integer_traits<long long>::const_max,
            integer_traits<unsigned long long>::const_max
        ),
        integer_traits<long long>::const_max
    );

    BOOST_CHECK_EQUAL(
        auto_min(
            integer_traits<long long>::const_min,
            integer_traits<unsigned long long>::const_max
        ),
        integer_traits<long long>::const_min
    );

    BOOST_CHECK_EQUAL(
        auto_min(
            integer_traits<long long>::const_max,
            integer_traits<unsigned long long>::const_min
        ),
        static_cast<long long>(0)
    );

    BOOST_CHECK_EQUAL(
        auto_min(static_cast<long long>(-1), 0xFFFFFFFFFFFFFFFEull),
        static_cast<long long>(-1)
    );
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("auto_min test");
    test->add(BOOST_TEST_CASE(&auto_min_result_test));
    test->add(BOOST_TEST_CASE(&auto_min_test));
    return test;
}
