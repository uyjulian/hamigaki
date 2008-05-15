// sha512_test.cpp: test case for SHA-512

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
std::string sha512checksum(const std::string& s)
{
    E cs;
    cs.process_bytes(s.c_str(), s.size());
    return hamigaki::to_hex<char>(cs.checksum(), false);
}

template<class E>
void sha512_test1()
{
    BOOST_CHECK_EQUAL(sha512checksum<E>("abc").size(), 128u);

    BOOST_CHECK_EQUAL(
        sha512checksum<E>("abc"),
        std::string(
            "ddaf35a193617abacc417349ae204131"
            "12e6fa4e89a97ea20a9eeee64b55d39a"
            "2192992a274fc1a836ba3c23a3feebbd"
            "454d4423643ce80e2a9ac94fa54ca49f"
        )
    );

    BOOST_CHECK_EQUAL(
        sha512checksum<E>(
            "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
            "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"
        ),
        std::string(
            "8e959b75dae313da8cf4f72814fc143f"
            "8f7779c6eb9f7fa17299aeadb6889018"
            "501d289e4900f7e4331b99dec4b5433a"
            "c7d329eeb6dd26545e96e55b874be909"
        )
    );
}

template<class E>
void sha512_test2()
{
    cksum::sha512 cs;
    for (int i = 0; i < 1000000; ++i)
        cs.process_byte(static_cast<unsigned char>('a'));
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest,
        std::string(
            "e718483d0ce769644e2e42c7bc15b463"
            "8e1f98b13b2044285632a803afa973eb"
            "de0ff244877ea60a4cb0432ce577c31b"
            "eb009c5c2c49aa2e4eadb217ad8cc09b"
        )
    );
}

template<class E>
void sha512_test3()
{
    const char data[] =
        "01234567012345670123456701234567"
        "01234567012345670123456701234567";

    cksum::sha512 cs;
    for (int i = 0; i < 10; ++i)
        cs.process_bytes(data, sizeof(data)-1);
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest,
        std::string(
            "89d05ba632c699c31231ded4ffc127d5"
            "a894dad412c0e024db872d1abd2ba814"
            "1a0f85072a9be1e2aa04cf33c765cb51"
            "0813a39cd5a84c4acaa64d3f3fb7bae9"
        )
    );
}

template<class E>
void sha512_test4()
{
    BOOST_CHECK_EQUAL(
        sha512checksum<E>(""),
        std::string(
            "cf83e1357eefb8bdf1542850d66d8007"
            "d620e4050b5715dc83f4a921d36ce9ce"
            "47d0d13c5d85f2b0ff8318d2877eec2f"
            "63b931bd47417a81a538327af927da3e"
        )
    );

    BOOST_CHECK_EQUAL(
        sha512checksum<E>(std::string(896u, 'a')),
        std::string(
            "e6837e6011b498390f99a683f6ffe6c8"
            "17ad1a8eed569ede7f5eea1f048f8670"
            "c5a2ffda032bc9024f64e719530ee363"
            "3ba15875ddd22841956ae400eb645015"
        )
    );
}

template<class E>
void sha512_test()
{
    sha512_test1<E>();
    sha512_test2<E>();
    sha512_test3<E>();
    sha512_test4<E>();
}

void sha512_basic_test()
{
    sha512_test<cksum::sha2_basic<512> >();
}

void sha512_optimal_test()
{
    sha512_test<cksum::sha512>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("SHA-512 test");
    test->add(BOOST_TEST_CASE(&sha512_basic_test));
    test->add(BOOST_TEST_CASE(&sha512_optimal_test));
    return test;
}
