// sha256_test.cpp: test case for SHA-256

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#include <hamigaki/checksum/sha256.hpp>
#include <hamigaki/hex_format.hpp>
#include <boost/test/unit_test.hpp>

namespace cksum = hamigaki::checksum;
namespace ut = boost::unit_test;

template<class E>
std::string sha256checksum(const std::string& s)
{
    E cs;
    cs.process_bytes(s.c_str(), s.size());
    return hamigaki::to_hex<char>(cs.checksum(), false);
}

template<class E>
void sha256_test1()
{
    BOOST_CHECK_EQUAL(sha256checksum<E>("abc").size(), 64u);

    BOOST_CHECK_EQUAL(
        sha256checksum<E>("abc"),
        std::string(
            "ba7816bf8f01cfea414140de5dae2223"
            "b00361a396177a9cb410ff61f20015ad")
    );

    BOOST_CHECK_EQUAL(
        sha256checksum<E>(
            "abcdbcdecdefdefgefghfghighijhi"
            "jkijkljklmklmnlmnomnopnopq"),
        std::string(
            "248d6a61d20638b8e5c026930c3e6039"
            "a33ce45964ff2167f6ecedd419db06c1")
    );
}

template<class E>
void sha256_test2()
{
    cksum::sha256 cs;
    for (int i = 0; i < 1000000; ++i)
        cs.process_byte(static_cast<unsigned char>('a'));
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest,
        std::string(
            "cdc76e5c9914fb9281a1c7e284d73e67"
            "f1809a48a497200e046d39ccc7112cd0")
    );
}

template<class E>
void sha256_test3()
{
    const char data[] =
        "01234567012345670123456701234567"
        "01234567012345670123456701234567";

    cksum::sha256 cs;
    for (int i = 0; i < 10; ++i)
        cs.process_bytes(data, sizeof(data)-1);
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest,
        std::string(
            "594847328451bdfa85056225462cc1d8"
            "67d877fb388df0ce35f25ab5562bfbb5")
    );
}

template<class E>
void sha256_test4()
{
    BOOST_CHECK_EQUAL(
        sha256checksum<E>(""),
        std::string(
            "e3b0c44298fc1c149afbf4c8996fb924"
            "27ae41e4649b934ca495991b7852b855")
    );

    BOOST_CHECK_EQUAL(
        sha256checksum<E>(std::string(448u, 'a')),
        std::string(
            "8984f047b9332de349e82f5f855bf0d2"
            "24bcec9b3c7f59f9ae022cad44119f65")
    );
}

void sha256_test()
{
    sha256_test1<cksum::sha256>();
    sha256_test2<cksum::sha256>();
    sha256_test3<cksum::sha256>();
    sha256_test4<cksum::sha256>();
}

void sha256_optimal_test()
{
    sha256_test1<cksum::sha256_optimal>();
    sha256_test2<cksum::sha256_optimal>();
    sha256_test3<cksum::sha256_optimal>();
    sha256_test4<cksum::sha256_optimal>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("SHA-256 test");
    test->add(BOOST_TEST_CASE(&sha256_test));
    test->add(BOOST_TEST_CASE(&sha256_optimal_test));
    return test;
}
