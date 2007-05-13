// md5_test.cpp: test case for MD5

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/checksum for library home page.

#include <hamigaki/checksum/md5.hpp>
#include <hamigaki/hex_format.hpp>
#include <boost/test/unit_test.hpp>

namespace cksum = hamigaki::checksum;
namespace ut = boost::unit_test;

std::string md5checksum(const std::string& s)
{
    cksum::md5 cs;
    cs.process_bytes(s.c_str(), s.size());
    return hamigaki::to_hex<char>(cs.checksum(), false);
}

void md5_test()
{
    BOOST_CHECK_EQUAL(md5checksum("").size(), 32u);

    BOOST_CHECK_EQUAL(
        md5checksum(""),
        std::string("d41d8cd98f00b204e9800998ecf8427e")
    );

    BOOST_CHECK_EQUAL(
        md5checksum("a"),
        std::string("0cc175b9c0f1b6a831c399e269772661")
    );

    BOOST_CHECK_EQUAL(
        md5checksum("abc"),
        std::string("900150983cd24fb0d6963f7d28e17f72")
    );

    BOOST_CHECK_EQUAL(
        md5checksum("message digest"),
        std::string("f96b697d7cb7938d525a2f31aaf161d0")
    );
    BOOST_CHECK_EQUAL(
        md5checksum("abcdefghijklmnopqrstuvwxyz"),
        std::string("c3fcd3d76192e4007dfb496cca67e13b")
    );

    BOOST_CHECK_EQUAL(
        md5checksum(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz0123456789"),
        std::string("d174ab98d277d9f5a5611c2c9f419d9f")
    );

    BOOST_CHECK_EQUAL(
        md5checksum(
            "1234567890123456789012345678901234567890"
            "1234567890123456789012345678901234567890"),
        std::string("57edf4a22be3c955ac49da2e2107b67a")
    );
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("md5 test");
    test->add(BOOST_TEST_CASE(&md5_test));
    return test;
}
