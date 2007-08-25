// binary_io_test.cpp: test case for binary_io.hpp

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/binary for library home page.

#include <hamigaki/binary/binary_io.hpp>
#include <boost/test/unit_test.hpp>
#include <cstring>
#include "./test_struct.hpp"

namespace ut = boost::unit_test;

struct inner
{
    boost::uint16_t big_val;
    boost::uint16_t lit_val;
    boost::uint16_t def_val;
};

struct outer
{
    boost::uint16_t big_val;
    boost::uint16_t lit_val;
    boost::uint16_t def_val;
    inner big_child;
    inner lit_child;
    inner def_child;
};

struct both
{
    boost::uint16_t val;
};

namespace hamigaki
{

template<>
struct struct_traits< ::inner>
{
private:
    typedef ::inner self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::big_val, big>,
        member<self, boost::uint16_t, &self::lit_val, little>,
        member<self, boost::uint16_t, &self::def_val, default_>
    > members;
};

template<>
struct struct_traits< ::outer>
{
private:
    typedef ::outer self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::big_val, big>,
        member<self, boost::uint16_t, &self::lit_val, little>,
        member<self, boost::uint16_t, &self::def_val, default_>,
        member<self, ::inner, &self::big_child, big>,
        member<self, ::inner, &self::lit_child, little>,
        member<self, ::inner, &self::def_child, default_>
    > members;
};

template<>
struct struct_traits< ::both>
{
private:
    typedef ::both self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::val, little>,
        member<self, boost::uint16_t, &self::val, big>
    > members;
};

} // namespace hamigaki

void binary_read_test()
{
    ::test_struct data;

    hamigaki::binary_read(
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67",
        data
    );

    BOOST_CHECK_EQUAL(data.u16, 0x2301);
    BOOST_CHECK(data.ca5[0] == '\x23');
    BOOST_CHECK(data.ca5[1] == '\x45');
    BOOST_CHECK(data.ca5[2] == '\x67');
    BOOST_CHECK(data.ca5[3] == '\x89');
    BOOST_CHECK(data.ca5[4] == '\xAB');
    BOOST_CHECK_EQUAL(data.u16a3[0], 0xCDEF);
    BOOST_CHECK_EQUAL(data.u16a3[1], 0x0123);
    BOOST_CHECK_EQUAL(data.u16a3[2], 0x4567);
    BOOST_CHECK_EQUAL(data.child.a, 0xEFCDAB89u);
    BOOST_CHECK_EQUAL(data.child.b, 0x01234567u);
}

void binary_write_test()
{
    ::test_struct data;
    data.u16 = 0x2301;
    data.ca5[0] = '\x23';
    data.ca5[1] = '\x45';
    data.ca5[2] = '\x67';
    data.ca5[3] = '\x89';
    data.ca5[4] = '\xAB';
    data.u16a3[0] = 0xCDEF;
    data.u16a3[1] = 0x0123;
    data.u16a3[2] = 0x4567;
    data.child.a = 0xEFCDAB89;
    data.child.b = 0x01234567;

    char buf[hamigaki::struct_size< ::test_struct>::value+1];
    std::memset(buf, 0xCC, sizeof(buf));
    hamigaki::binary_write(buf, data);

    const char s[] =
        "\x01\x23\x00\x00\x00\x00\x00\x00"
        "\x00\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\xCC";

    BOOST_WARN_EQUAL_COLLECTIONS(buf, buf+sizeof(buf), s, s+sizeof(s)-1);
}

void default_read_test()
{
    ::outer data;

    hamigaki::binary_read<hamigaki::big>(
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF",
        data
    );

    BOOST_CHECK_EQUAL(data.big_val, 0x0123);
    BOOST_CHECK_EQUAL(data.lit_val, 0x6745);
    BOOST_CHECK_EQUAL(data.def_val, 0x89AB);
    BOOST_CHECK_EQUAL(data.big_child.big_val, 0xCDEF);
    BOOST_CHECK_EQUAL(data.big_child.lit_val, 0x2301);
    BOOST_CHECK_EQUAL(data.big_child.def_val, 0x4567);
    BOOST_CHECK_EQUAL(data.lit_child.big_val, 0x89AB);
    BOOST_CHECK_EQUAL(data.lit_child.lit_val, 0xEFCD);
    BOOST_CHECK_EQUAL(data.lit_child.def_val, 0x2301);
    BOOST_CHECK_EQUAL(data.def_child.big_val, 0x4567);
    BOOST_CHECK_EQUAL(data.def_child.lit_val, 0xAB89);
    BOOST_CHECK_EQUAL(data.def_child.def_val, 0xCDEF);


    hamigaki::binary_read<hamigaki::little>(
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
        "\x01\x23\x45\x67\x89\xAB\xCD\xEF",
        data
    );

    BOOST_CHECK_EQUAL(data.big_val, 0x0123);
    BOOST_CHECK_EQUAL(data.lit_val, 0x6745);
    BOOST_CHECK_EQUAL(data.def_val, 0xAB89);
    BOOST_CHECK_EQUAL(data.big_child.big_val, 0xCDEF);
    BOOST_CHECK_EQUAL(data.big_child.lit_val, 0x2301);
    BOOST_CHECK_EQUAL(data.big_child.def_val, 0x4567);
    BOOST_CHECK_EQUAL(data.lit_child.big_val, 0x89AB);
    BOOST_CHECK_EQUAL(data.lit_child.lit_val, 0xEFCD);
    BOOST_CHECK_EQUAL(data.lit_child.def_val, 0x2301);
    BOOST_CHECK_EQUAL(data.def_child.big_val, 0x4567);
    BOOST_CHECK_EQUAL(data.def_child.lit_val, 0xAB89);
    BOOST_CHECK_EQUAL(data.def_child.def_val, 0xEFCD);
}

void default_write_test()
{
    ::outer data;
    data.big_val = 0x0123;
    data.lit_val = 0x4567;
    data.def_val = 0x89AB;
    data.big_child.big_val = 0xCDEF;
    data.big_child.lit_val = 0x0123;
    data.big_child.def_val = 0x4567;
    data.lit_child.big_val = 0x89AB;
    data.lit_child.lit_val = 0xCDEF;
    data.lit_child.def_val = 0x0123;
    data.def_child.big_val = 0x4567;
    data.def_child.lit_val = 0x89AB;
    data.def_child.def_val = 0xCDEF;

    char buf[hamigaki::struct_size< ::outer>::value+1];
    std::memset(buf, 0xCC, sizeof(buf));
    hamigaki::binary_write<hamigaki::big>(buf, data);

    const char bs[] =
        "\x01\x23\x67\x45\x89\xAB\xCD\xEF"
        "\x23\x01\x45\x67\x89\xAB\xEF\xCD"
        "\x23\x01\x45\x67\xAB\x89\xCD\xEF"
        "\xCC";

    BOOST_WARN_EQUAL_COLLECTIONS(buf, buf+sizeof(buf), bs, bs+sizeof(bs)-1);

    std::memset(buf, 0xCC, sizeof(buf));
    hamigaki::binary_write<hamigaki::little>(buf, data);

    const char ls[] =
        "\x01\x23\x67\x45\xAB\x89\xCD\xEF"
        "\x23\x01\x45\x67\x89\xAB\xEF\xCD"
        "\x23\x01\x45\x67\xAB\x89\xEF\xCD"
        "\xCC";

    BOOST_WARN_EQUAL_COLLECTIONS(buf, buf+sizeof(buf), ls, ls+sizeof(ls)-1);
}

void both_test()
{
    ::both data;
    data.val = 0x0123;

    char buf[hamigaki::struct_size< ::both>::value+1];
    std::memset(buf, 0xCC, sizeof(buf));
    hamigaki::binary_write(buf, data);

    const char s[] = "\x23\x01\01\x23\xCC";

    BOOST_WARN_EQUAL_COLLECTIONS(buf, buf+sizeof(buf), s, s+sizeof(s)-1);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("binary I/O test");
    test->add(BOOST_TEST_CASE(&binary_read_test));
    test->add(BOOST_TEST_CASE(&binary_write_test));
    test->add(BOOST_TEST_CASE(&default_read_test));
    test->add(BOOST_TEST_CASE(&default_write_test));
    test->add(BOOST_TEST_CASE(&both_test));
    return test;
}
