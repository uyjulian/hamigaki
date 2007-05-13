// sum8_test.cpp: test case for xor8

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#include <hamigaki/checksum/xor8.hpp>
#include <boost/test/unit_test.hpp>

namespace cksum = hamigaki::checksum;
namespace ut = boost::unit_test;

void process_byte_test()
{
    const unsigned char data[] = { 0x25, 0x62, 0x3F, 0x52 };
    cksum::xor8 cs;
    for (std::size_t i = 0; i < sizeof(data); ++i)
        cs.process_byte(data[i]);
    BOOST_CHECK_EQUAL(cs.checksum(), static_cast<unsigned char>(0x2A));
}

void process_block_test()
{
    const unsigned char data[] = { 0x25, 0x62, 0x3F, 0x52 };
    cksum::xor8 cs;
    cs.process_block(data, data + sizeof(data));
    BOOST_CHECK_EQUAL(cs.checksum(), static_cast<unsigned char>(0x2A));
}

void process_bytes_test()
{
    const unsigned char data[] = { 0x25, 0x62, 0x3F, 0x52 };
    cksum::xor8 cs;
    cs.process_bytes(data, sizeof(data));
    BOOST_CHECK_EQUAL(cs.checksum(), static_cast<unsigned char>(0x2A));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("xor8 test");
    test->add(BOOST_TEST_CASE(&process_byte_test));
    test->add(BOOST_TEST_CASE(&process_block_test));
    test->add(BOOST_TEST_CASE(&process_bytes_test));
    return test;
}
