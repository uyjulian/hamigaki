// endian_test.cpp: test case for endian utility

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/binary for library home page.

#include <hamigaki/binary/endian.hpp>
#include <boost/test/unit_test.hpp>
#include <cstring>

namespace ut = boost::unit_test;

using hamigaki::big;
using hamigaki::little;
using hamigaki::encode_uint;
using hamigaki::decode_uint;
using hamigaki::encode_int;
using hamigaki::decode_int;

void unsigned_big_endian_test()
{
    char buf[8+1];

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<big,1>(buf, 0x12);
    BOOST_CHECK(buf[0] == '\x12');
    BOOST_CHECK(buf[1] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<big,2>(buf, 0x4321);
    BOOST_CHECK(buf[0] == '\x43');
    BOOST_CHECK(buf[1] == '\x21');
    BOOST_CHECK(buf[2] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<big,3>(buf, 0x123456);
    BOOST_CHECK(buf[0] == '\x12');
    BOOST_CHECK(buf[1] == '\x34');
    BOOST_CHECK(buf[2] == '\x56');
    BOOST_CHECK(buf[3] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<big,4>(buf, 0x87654321);
    BOOST_CHECK(buf[0] == '\x87');
    BOOST_CHECK(buf[1] == '\x65');
    BOOST_CHECK(buf[2] == '\x43');
    BOOST_CHECK(buf[3] == '\x21');
    BOOST_CHECK(buf[4] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<big,8>(buf, 0x0123456789ABCDEFull);
    BOOST_CHECK(buf[0] == '\x01');
    BOOST_CHECK(buf[1] == '\x23');
    BOOST_CHECK(buf[2] == '\x45');
    BOOST_CHECK(buf[3] == '\x67');
    BOOST_CHECK(buf[4] == '\x89');
    BOOST_CHECK(buf[5] == '\xAB');
    BOOST_CHECK(buf[6] == '\xCD');
    BOOST_CHECK(buf[7] == '\xEF');
    BOOST_CHECK(buf[8] == '\xCC');


    BOOST_CHECK((decode_uint<big,1>("\x12") == 0x12));
    BOOST_CHECK((decode_uint<big,2>("\x43\x21") == 0x4321));
    BOOST_CHECK((decode_uint<big,3>("\x12\x34\x56") == 0x123456));
    BOOST_CHECK((decode_uint<big,4>("\x87\x65\x43\x21") == 0x87654321));
    BOOST_CHECK_EQUAL(
        (decode_uint<big,8>("\x01\x23\x45\x67\x89\xAB\xCD\xEF")),
        0x0123456789ABCDEFull
    );
}

void unsigned_little_endian_test()
{
    char buf[8+1];

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<little,1>(buf, 0x12);
    BOOST_CHECK(buf[0] == '\x12');
    BOOST_CHECK(buf[1] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<little,2>(buf, 0x4321);
    BOOST_CHECK(buf[0] == '\x21');
    BOOST_CHECK(buf[1] == '\x43');
    BOOST_CHECK(buf[2] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<little,3>(buf, 0x123456);
    BOOST_CHECK(buf[0] == '\x56');
    BOOST_CHECK(buf[1] == '\x34');
    BOOST_CHECK(buf[2] == '\x12');
    BOOST_CHECK(buf[3] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<little,4>(buf, 0x87654321);
    BOOST_CHECK(buf[0] == '\x21');
    BOOST_CHECK(buf[1] == '\x43');
    BOOST_CHECK(buf[2] == '\x65');
    BOOST_CHECK(buf[3] == '\x87');
    BOOST_CHECK(buf[4] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_uint<little,8>(buf, 0x0123456789ABCDEFull);
    BOOST_CHECK(buf[0] == '\xEF');
    BOOST_CHECK(buf[1] == '\xCD');
    BOOST_CHECK(buf[2] == '\xAB');
    BOOST_CHECK(buf[3] == '\x89');
    BOOST_CHECK(buf[4] == '\x67');
    BOOST_CHECK(buf[5] == '\x45');
    BOOST_CHECK(buf[6] == '\x23');
    BOOST_CHECK(buf[7] == '\x01');
    BOOST_CHECK(buf[8] == '\xCC');


    BOOST_CHECK((decode_uint<little,1>("\x12") == 0x12));
    BOOST_CHECK((decode_uint<little,2>("\x43\x21") == 0x2143));
    BOOST_CHECK((decode_uint<little,3>("\x12\x34\x56") == 0x563412));
    BOOST_CHECK((decode_uint<little,4>("\x87\x65\x43\x21") == 0x21436587));
    BOOST_CHECK_EQUAL(
        (decode_uint<little,8>("\x01\x23\x45\x67\x89\xAB\xCD\xEF")),
        0xEFCDAB8967452301ull
    );
}

void signed_big_endian_test()
{
    char buf[8+1];

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<big,1>(buf, static_cast<boost::int8_t>(-123));
    BOOST_CHECK(buf[0] == '\x85');
    BOOST_CHECK(buf[1] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<big,2>(buf, static_cast<boost::int16_t>(-12345));
    BOOST_CHECK(buf[0] == '\xCF');
    BOOST_CHECK(buf[1] == '\xC7');
    BOOST_CHECK(buf[2] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<big,3>(buf, -1234567);
    BOOST_CHECK(buf[0] == '\xED');
    BOOST_CHECK(buf[1] == '\x29');
    BOOST_CHECK(buf[2] == '\x79');
    BOOST_CHECK(buf[3] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<big,4>(buf, -1234567890);
    BOOST_CHECK(buf[0] == '\xB6');
    BOOST_CHECK(buf[1] == '\x69');
    BOOST_CHECK(buf[2] == '\xFD');
    BOOST_CHECK(buf[3] == '\x2E');
    BOOST_CHECK(buf[4] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<big,8>(buf, -1234567890123456789ll);
    BOOST_CHECK(buf[0] == '\xEE');
    BOOST_CHECK(buf[1] == '\xDD');
    BOOST_CHECK(buf[2] == '\xEF');
    BOOST_CHECK(buf[3] == '\x0B');
    BOOST_CHECK(buf[4] == '\x82');
    BOOST_CHECK(buf[5] == '\x16');
    BOOST_CHECK(buf[6] == '\x7E');
    BOOST_CHECK(buf[7] == '\xEB');
    BOOST_CHECK(buf[8] == '\xCC');


    BOOST_CHECK(
        (decode_int<big,1>("\x85") == static_cast<boost::int8_t>(-123))
    );
    BOOST_CHECK_EQUAL(
        (decode_int<big,2>("\xCF\xC7")),
        static_cast<boost::int16_t>(-12345)
    );
    BOOST_CHECK_EQUAL(
        (decode_int<big,3>("\xED\x29\x79")),
        -1234567
    );
    BOOST_CHECK_EQUAL(
        (decode_int<big,4>("\xB6\x69\xFD\x2E")),
        -1234567890
    );
    BOOST_CHECK_EQUAL(
        (decode_int<big,8>("\xEE\xDD\xEF\x0B\x82\x16\x7E\xEB")),
        -1234567890123456789ll
    );
}

void signed_little_endian_test()
{
    char buf[8+1];

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<little,1>(buf, static_cast<boost::int8_t>(-123));
    BOOST_CHECK(buf[0] == '\x85');
    BOOST_CHECK(buf[1] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<little,2>(buf, static_cast<boost::int16_t>(-12345));
    BOOST_CHECK(buf[0] == '\xC7');
    BOOST_CHECK(buf[1] == '\xCF');
    BOOST_CHECK(buf[2] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<little,3>(buf, -1234567);
    BOOST_CHECK(buf[0] == '\x79');
    BOOST_CHECK(buf[1] == '\x29');
    BOOST_CHECK(buf[2] == '\xED');
    BOOST_CHECK(buf[3] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<little,4>(buf, -1234567890);
    BOOST_CHECK(buf[0] == '\x2E');
    BOOST_CHECK(buf[1] == '\xFD');
    BOOST_CHECK(buf[2] == '\x69');
    BOOST_CHECK(buf[3] == '\xB6');
    BOOST_CHECK(buf[4] == '\xCC');

    std::memset(buf, 0xCC, sizeof(buf));
    encode_int<little,8>(buf, -1234567890123456789ll);
    BOOST_CHECK(buf[0] == '\xEB');
    BOOST_CHECK(buf[1] == '\x7E');
    BOOST_CHECK(buf[2] == '\x16');
    BOOST_CHECK(buf[3] == '\x82');
    BOOST_CHECK(buf[4] == '\x0B');
    BOOST_CHECK(buf[5] == '\xEF');
    BOOST_CHECK(buf[6] == '\xDD');
    BOOST_CHECK(buf[7] == '\xEE');
    BOOST_CHECK(buf[8] == '\xCC');


    BOOST_CHECK(
        (decode_int<little,1>("\x85") == static_cast<boost::int8_t>(-123))
    );
    BOOST_CHECK_EQUAL(
        (decode_int<little,2>("\xC7\xCF")),
        static_cast<boost::int16_t>(-12345)
    );
    BOOST_CHECK_EQUAL(
        (decode_int<little,3>("\x79\x29\xED")),
        -1234567
    );
    BOOST_CHECK_EQUAL(
        (decode_int<little,4>("\x2E\xFD\x69\xB6")),
        -1234567890
    );
    BOOST_CHECK_EQUAL(
        (decode_int<little,8>("\xEB\x7E\x16\x82\x0B\xEF\xDD\xEE")),
        -1234567890123456789ll
    );
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("endian test");
    test->add(BOOST_TEST_CASE(&unsigned_big_endian_test));
    test->add(BOOST_TEST_CASE(&unsigned_little_endian_test));
    test->add(BOOST_TEST_CASE(&signed_big_endian_test));
    test->add(BOOST_TEST_CASE(&signed_little_endian_test));
    return test;
}
