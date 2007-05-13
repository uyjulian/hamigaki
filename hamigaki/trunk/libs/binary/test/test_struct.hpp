// test_struct.hpp: struct for test

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/binary for library home page.

#ifndef HAMIGAKI_BINARY_TEST_STRUCT_HPP
#define HAMIGAKI_BINARY_TEST_STRUCT_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

struct child_struct
{
    boost::uint32_t a;
    boost::uint32_t b;
};

struct test_struct
{
    boost::uint16_t u16;
    char ca5[5];
    boost::uint16_t u16a3[3];
    child_struct child;
};

namespace hamigaki
{

template<>
struct struct_traits< ::child_struct>
{
private:
    typedef ::child_struct self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint32_t, &self::a, little>,
        member<self, boost::uint32_t, &self::b, big>
    > members;
};

template<>
struct struct_traits< ::test_struct>
{
private:
    typedef ::test_struct self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::u16, little>,
        padding<7>,
        member<self, char[5], &self::ca5>,
        member<self, boost::uint16_t[3], &self::u16a3, big>,
        member<self, ::child_struct, &self::child>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_BINARY_TEST_STRUCT_HPP
