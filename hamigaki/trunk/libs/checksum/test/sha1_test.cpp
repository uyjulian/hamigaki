// sha1_test.cpp: test case for SHA-1

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#include <hamigaki/checksum/sha1.hpp>
#include <hamigaki/hex_format.hpp>
#include <boost/test/unit_test.hpp>

namespace cksum = hamigaki::checksum;
namespace ut = boost::unit_test;

template<class E>
std::string sha1checksum(const std::string& s)
{
    E cs;
    cs.process_bytes(s.c_str(), s.size());
    return hamigaki::to_hex<char>(cs.checksum(), false);
}

template<class E>
void sha1_test1()
{
    BOOST_CHECK_EQUAL(sha1checksum<E>("abc").size(), 40u);

    BOOST_CHECK_EQUAL(
        sha1checksum<E>("abc"),
        std::string("a9993e364706816aba3e25717850c26c9cd0d89d")
    );

    BOOST_CHECK_EQUAL(
        sha1checksum<E>(
            "abcdbcdecdefdefgefghfghighijhi"
            "jkijkljklmklmnlmnomnopnopq"),
        std::string("84983e441c3bd26ebaae4aa1f95129e5e54670f1")
    );
}

template<class E>
void sha1_test2()
{
    cksum::sha1 cs;
    for (int i = 0; i < 1000000; ++i)
        cs.process_byte(static_cast<unsigned char>('a'));
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest, std::string("34aa973cd4c4daa4f61eeb2bdbad27316534016f"));
}

template<class E>
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

template<class E>
void sha1_test4()
{
    BOOST_CHECK_EQUAL(
        sha1checksum<E>(""),
        std::string("da39a3ee5e6b4b0d3255bfef95601890afd80709")
    );

    BOOST_CHECK_EQUAL(
        sha1checksum<E>(std::string(448u, 'a')),
        std::string("7c6d6ed601becc0a68dbca91800a634cd1e5e5df")
    );
}

void sha1_test()
{
    sha1_test1<cksum::sha1>();
    sha1_test2<cksum::sha1>();
    sha1_test3<cksum::sha1>();
    sha1_test4<cksum::sha1>();
}

void sha1_optimal_test()
{
    sha1_test1<cksum::sha1_optimal>();
    sha1_test2<cksum::sha1_optimal>();
    sha1_test3<cksum::sha1_optimal>();
    sha1_test4<cksum::sha1_optimal>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("SHA-1 test");
    test->add(BOOST_TEST_CASE(&sha1_test));
    test->add(BOOST_TEST_CASE(&sha1_optimal_test));
    return test;
}
