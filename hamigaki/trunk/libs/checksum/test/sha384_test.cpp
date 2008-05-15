// sha384_test.cpp: test case for SHA-384

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#include <hamigaki/checksum/sha2.hpp>
#include <hamigaki/hex_format.hpp>
#include <boost/test/unit_test.hpp>

namespace cksum = hamigaki::checksum;
namespace ut = boost::unit_test;

template<class E>
std::string sha384checksum(const std::string& s)
{
    E cs;
    cs.process_bytes(s.c_str(), s.size());
    return hamigaki::to_hex<char>(cs.checksum(), false);
}

template<class E>
void sha384_test1()
{
    BOOST_CHECK_EQUAL(sha384checksum<E>("abc").size(), 96u);

    BOOST_CHECK_EQUAL(
        sha384checksum<E>("abc"),
        std::string(
            "cb00753f45a35e8bb5a03d699ac65007"
            "272c32ab0eded1631a8b605a43ff5bed"
            "8086072ba1e7cc2358baeca134c825a7"
        )
    );

    BOOST_CHECK_EQUAL(
        sha384checksum<E>(
            "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
            "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"
        ),
        std::string(
            "09330c33f71147e83d192fc782cd1b47"
            "53111b173b3b05d22fa08086e3b0f712"
            "fcc7c71a557e2db966c3e9fa91746039"
        )
    );
}

template<class E>
void sha384_test2()
{
    cksum::sha384 cs;
    for (int i = 0; i < 1000000; ++i)
        cs.process_byte(static_cast<unsigned char>('a'));
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest,
        std::string(
            "9d0e1809716474cb086e834e310a4a1c"
            "ed149e9c00f248527972cec5704c2a5b"
            "07b8b3dc38ecc4ebae97ddd87f3d8985"
        )
    );
}

template<class E>
void sha384_test3()
{
    const char data[] =
        "01234567012345670123456701234567"
        "01234567012345670123456701234567";

    cksum::sha384 cs;
    for (int i = 0; i < 10; ++i)
        cs.process_bytes(data, sizeof(data)-1);
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest,
        std::string(
            "2fc64a4f500ddb6828f6a3430b8dd72a"
            "368eb7f3a8322a70bc84275b9c0b3ab0"
            "0d27a5cc3c2d224aa6b61a0d79fb4596"
        )
    );
}

template<class E>
void sha384_test4()
{
    BOOST_CHECK_EQUAL(
        sha384checksum<E>(""),
        std::string(
            "38b060a751ac96384cd9327eb1b1e36a"
            "21fdb71114be07434c0cc7bf63f6e1da"
            "274edebfe76f65fbd51ad2f14898b95b"
        )
    );

    BOOST_CHECK_EQUAL(
        sha384checksum<E>(std::string(896u, 'a')),
        std::string(
            "dbd662447ac432708a7654e702cf726d"
            "dabb39ef3668e21d995d0c122e050c4a"
            "93dd71507af98b31f084c7d00909f68a"
        )
    );
}

template<class E>
void sha384_test()
{
    sha384_test1<E>();
    sha384_test2<E>();
    sha384_test3<E>();
    sha384_test4<E>();
}

void sha384_basic_test()
{
    sha384_test<cksum::sha2_basic<384> >();
}

void sha384_optimal_test()
{
    sha384_test<cksum::sha384>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("SHA-384 test");
    test->add(BOOST_TEST_CASE(&sha384_basic_test));
    test->add(BOOST_TEST_CASE(&sha384_optimal_test));
    return test;
}
