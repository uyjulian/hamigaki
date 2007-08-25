// sha1_test.cpp: test case for SHA-1

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#include <hamigaki/checksum/sha1.hpp>
#include <hamigaki/hex_format.hpp>
#include <boost/test/unit_test.hpp>

namespace cksum = hamigaki::checksum;
namespace ut = boost::unit_test;

std::string sha1checksum(const std::string& s)
{
    cksum::sha1 cs;
    cs.process_bytes(s.c_str(), s.size());
    return hamigaki::to_hex<char>(cs.checksum(), false);
}

void sha1_test()
{
    BOOST_CHECK_EQUAL(sha1checksum("abc").size(), 40u);

    BOOST_CHECK_EQUAL(
        sha1checksum("abc"),
        std::string("a9993e364706816aba3e25717850c26c9cd0d89d")
    );

    BOOST_CHECK_EQUAL(
        sha1checksum(
            "abcdbcdecdefdefgefghfghighijhi"
            "jkijkljklmklmnlmnomnopnopq"),
        std::string("84983e441c3bd26ebaae4aa1f95129e5e54670f1")
    );

    BOOST_CHECK_EQUAL(
        sha1checksum(""),
        std::string("da39a3ee5e6b4b0d3255bfef95601890afd80709")
    );
}

void sha1_test2()
{
    cksum::sha1 cs;
    for (int i = 0; i < 1000000; ++i)
        cs.process_byte(static_cast<unsigned char>('a'));
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest, std::string("34aa973cd4c4daa4f61eeb2bdbad27316534016f"));
}

void sha1_test3()
{
    const char data[] =
        "01234567012345670123456701234567"
        "01234567012345670123456701234567";

    cksum::sha1 cs;
    for (int i = 0; i < 10; ++i)
        cs.process_bytes(data, sizeof(data)-1);
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest, std::string("dea356a2cddd90c7a7ecedc5ebb563934f460452"));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("SHA-1 test");
    test->add(BOOST_TEST_CASE(&sha1_test));
    test->add(BOOST_TEST_CASE(&sha1_test2));
    test->add(BOOST_TEST_CASE(&sha1_test3));
    return test;
}
