// sha224_test.cpp: test case for SHA-224

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
std::string sha224checksum(const std::string& s)
{
    E cs;
    cs.process_bytes(s.c_str(), s.size());
    return hamigaki::to_hex<char>(cs.checksum(), false);
}

template<class E>
void sha224_test1()
{
    BOOST_CHECK_EQUAL(sha224checksum<E>("abc").size(), 56u);

    BOOST_CHECK_EQUAL(
        sha224checksum<E>("abc"),
        std::string("23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7")
    );

    BOOST_CHECK_EQUAL(
        sha224checksum<E>(
            "abcdbcdecdefdefgefghfghighijhi"
            "jkijkljklmklmnlmnomnopnopq"),
        std::string("75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525")
    );
}

template<class E>
void sha224_test2()
{
    cksum::sha224 cs;
    for (int i = 0; i < 1000000; ++i)
        cs.process_byte(static_cast<unsigned char>('a'));
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest,
        std::string("20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67")
    );
}

template<class E>
void sha224_test3()
{
    const char data[] =
        "01234567012345670123456701234567"
        "01234567012345670123456701234567";

    cksum::sha224 cs;
    for (int i = 0; i < 10; ++i)
        cs.process_bytes(data, sizeof(data)-1);
    const std::string& digest = hamigaki::to_hex<char>(cs.checksum(), false);

    BOOST_CHECK_EQUAL(
        digest,
        std::string("567f69f168cd7844e65259ce658fe7aadfa25216e68eca0eb7ab8262")
    );
}

template<class E>
void sha224_test4()
{
    BOOST_CHECK_EQUAL(
        sha224checksum<E>(""),
        std::string("d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f")
    );

    BOOST_CHECK_EQUAL(
        sha224checksum<E>(std::string(448u, 'a')),
        std::string("cfcb1d9b09f53d26ac05712c3d4fec68554e3737f5cc6e738c6c7e9f")
    );
}

template<class E>
void sha224_test()
{
    sha224_test1<E>();
    sha224_test2<E>();
    sha224_test3<E>();
    sha224_test4<E>();
}

void sha224_basic_test()
{
    sha224_test<cksum::sha2_basic<224> >();
}

void sha224_optimal_test()
{
    sha224_test<cksum::sha224>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("SHA-224 test");
    test->add(BOOST_TEST_CASE(&sha224_basic_test));
    test->add(BOOST_TEST_CASE(&sha224_optimal_test));
    return test;
}
