// struct_traits_test.cpp: test case for struct_traits.hpp

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/binary for library home page.

#include <boost/mpl/assert.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/type_traits/is_same.hpp>
#include "./test_struct.hpp"

namespace ut = boost::unit_test;

void member_size_test()
{
    using hamigaki::big;
    using hamigaki::little;

    typedef ::test_struct self;

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::member_size<
                hamigaki::member<self, boost::uint16_t, &self::u16, little>
            >::type,
            boost::mpl::size_t<2>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::member_size<
                hamigaki::padding<7>
            >::type,
            boost::mpl::size_t<7>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::member_size<
                hamigaki::member<self, char[5], &self::ca5>
            >::type,
            boost::mpl::size_t<5>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::member_size<
                hamigaki::member<self, boost::uint16_t[3], &self::u16a3, big>
            >::type,
            boost::mpl::size_t<6>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::member_size<
                hamigaki::member<self, ::child_struct, &self::child>
            >::type,
            boost::mpl::size_t<8>
        >
    ));
}

void member_offset_test()
{
    using hamigaki::big;
    using hamigaki::little;

    typedef ::test_struct self;

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::member_offset<
                hamigaki::member<self, boost::uint16_t, &self::u16, little>
            >::type,
            boost::mpl::size_t<0>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::member_offset<
                hamigaki::member<self, char[5], &self::ca5>
            >::type,
            boost::mpl::size_t<9>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::member_offset<
                hamigaki::member<self, boost::uint16_t[3], &self::u16a3, big>
            >::type,
            boost::mpl::size_t<14>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::member_offset<
                hamigaki::member<self, ::child_struct, &self::child>
            >::type,
            boost::mpl::size_t<20>
        >
    ));
}

void struct_size_test()
{
    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::struct_size< ::child_struct>::type,
            boost::mpl::size_t<8>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::struct_size< ::test_struct>::type,
            boost::mpl::size_t<28>
        >
    ));
}

void binary_size_test()
{
    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::binary_size< ::child_struct>::type,
            boost::mpl::size_t<8>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::binary_size< ::test_struct>::type,
            boost::mpl::size_t<28>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::binary_size<boost::uint32_t>::type,
            boost::mpl::size_t<4>
        >
    ));
}

void binary_offset_test()
{
    using hamigaki::big;
    using hamigaki::little;

    typedef ::test_struct self;

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::binary_offset<
                self, boost::uint16_t, &self::u16
            >::type,
            boost::mpl::size_t<0>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::binary_offset<
                self, char[5], &self::ca5
            >::type,
            boost::mpl::size_t<9>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::binary_offset<
                self, boost::uint16_t[3], &self::u16a3
            >::type,
            boost::mpl::size_t<14>
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_same<
            hamigaki::binary_offset<
                self, ::child_struct, &self::child
            >::type,
            boost::mpl::size_t<20>
        >
    ));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("struct traits test");
    test->add(BOOST_TEST_CASE(&member_size_test));
    test->add(BOOST_TEST_CASE(&member_offset_test));
    test->add(BOOST_TEST_CASE(&struct_size_test));
    test->add(BOOST_TEST_CASE(&binary_size_test));
    test->add(BOOST_TEST_CASE(&binary_offset_test));
    return test;
}
