// utf16.cpp: test cases for utf16.hpp

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/charset for library home page.

#include <hamigaki/charset/utf16.hpp>
#include <boost/test/unit_test.hpp>

namespace charset = hamigaki::charset;
namespace ut = boost::unit_test;

template<std::size_t N>
struct size_tag {};

void encode_test_impl(const size_tag<4>&)
{
    charset::wstring ws(L"\U00029E3D");
    BOOST_CHECK_EQUAL(charset::to_utf16be(ws), "\xD8\x67\xDE\x3D");
    BOOST_CHECK_EQUAL(charset::to_utf16le(ws), "\x67\xD8\x3D\xDE");

    charset::wstring ws2(L"\U00029E3D.txt");

    const std::string& be1 = charset::to_utf16be(ws2);
    const std::string be2("\xD8\x67\xDE\x3D\0.\0t\0x\0t", 12);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        be1.begin(), be1.end(), be2.begin(), be2.end());

    const std::string& le1 = charset::to_utf16le(ws2);
    const std::string le2("\x67\xD8\x3D\xDE.\0t\0x\0t\0", 12);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        le1.begin(), le1.end(), le2.begin(), le2.end());
}

void encode_test_impl(const size_tag<2>&)
{
    charset::wstring ws(L"\xD867\xDE3D");
    BOOST_CHECK_EQUAL(charset::to_utf16be(ws), "\xD8\x67\xDE\x3D");
    BOOST_CHECK_EQUAL(charset::to_utf16le(ws), "\x67\xD8\x3D\xDE");

    charset::wstring ws2(L"\xD867\xDE3D.txt");

    const std::string& be1 = charset::to_utf16be(ws2);
    const std::string be2("\xD8\x67\xDE\x3D\0.\0t\0x\0t", 12);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        be1.begin(), be1.end(), be2.begin(), be2.end());

    const std::string& le1 = charset::to_utf16le(ws2);
    const std::string le2("\x67\xD8\x3D\xDE.\0t\0x\0t\0", 12);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        le1.begin(), le1.end(), le2.begin(), le2.end());
}

void encode_test()
{
    encode_test_impl((size_tag<sizeof(wchar_t)>()));
}

void decode_test_impl(const size_tag<4>&)
{
    const charset::wstring ws(L"\U00029E3D");
    BOOST_CHECK(charset::from_utf16be("\xD8\x67\xDE\x3D") == ws);
    BOOST_CHECK(charset::from_utf16le("\x67\xD8\x3D\xDE") == ws);

    const charset::wstring ws2(L"\U00029E3D.txt");
    const std::string be2("\xD8\x67\xDE\x3D\0.\0t\0x\0t", 12);
    const std::string le2("\x67\xD8\x3D\xDE.\0t\0x\0t\0", 12);

    BOOST_CHECK(charset::from_utf16be(be2) == ws2);
    BOOST_CHECK(charset::from_utf16le(le2) == ws2);
}

void decode_test_impl(const size_tag<2>&)
{
    const charset::wstring ws(L"\xD867\xDE3D");
    BOOST_CHECK(charset::from_utf16be("\xD8\x67\xDE\x3D") == ws);
    BOOST_CHECK(charset::from_utf16le("\x67\xD8\x3D\xDE") == ws);

    const charset::wstring ws2(L"\xD867\xDE3D.txt");
    const std::string be2("\xD8\x67\xDE\x3D\0.\0t\0x\0t", 12);
    const std::string le2("\x67\xD8\x3D\xDE.\0t\0x\0t\0", 12);

    BOOST_CHECK(charset::from_utf16be(be2) == ws2);
    BOOST_CHECK(charset::from_utf16le(le2) == ws2);
}

void decode_test()
{
    decode_test_impl((size_tag<sizeof(wchar_t)>()));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("code page test");
    test->add(BOOST_TEST_CASE(&encode_test));
    test->add(BOOST_TEST_CASE(&decode_test));
    return test;
}
