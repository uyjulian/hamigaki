// utf8.cpp: test cases for utf8.hpp

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/charset for library home page.

#include <hamigaki/charset/utf8.hpp>
#include <boost/test/unit_test.hpp>

namespace charset = hamigaki::charset;
namespace ut = boost::unit_test;

template<std::size_t N>
struct size_tag {};

void encode_test_impl(const size_tag<4>&)
{
    charset::wstring ws(L"\U00029E3D");
    BOOST_CHECK_EQUAL(charset::to_utf8(ws), "\xF0\xA9\xB8\xBD");

    charset::wstring ws2(L"\U00029E3D.txt");
    BOOST_CHECK_EQUAL(charset::to_utf8(ws2), "\xF0\xA9\xB8\xBD.txt");
}

void encode_test_impl(const size_tag<2>&)
{
    charset::wstring ws(L"\xD867\xDE3D");
    BOOST_CHECK_EQUAL(charset::to_utf8(ws), "\xF0\xA9\xB8\xBD");

    charset::wstring ws2(L"\xD867\xDE3D.txt");
    BOOST_CHECK_EQUAL(charset::to_utf8(ws2), "\xF0\xA9\xB8\xBD.txt");
}

void encode_test()
{
    encode_test_impl((size_tag<sizeof(wchar_t)>()));
}

void decode_test_impl(const size_tag<4>&)
{
    const charset::wstring ws(L"\U00029E3D");
    BOOST_CHECK(charset::from_utf8("\xF0\xA9\xB8\xBD") == ws);

    const charset::wstring ws2(L"\U00029E3D.txt");
    BOOST_CHECK(charset::from_utf8("\xF0\xA9\xB8\xBD.txt") == ws2);
}

void decode_test_impl(const size_tag<2>&)
{
    const charset::wstring ws(L"\xD867\xDE3D");
    BOOST_CHECK(charset::from_utf8("\xF0\xA9\xB8\xBD") == ws);

    const charset::wstring ws2(L"\xD867\xDE3D.txt");
    BOOST_CHECK(charset::from_utf8("\xF0\xA9\xB8\xBD.txt") == ws2);
}

void decode_test()
{
    decode_test_impl((size_tag<sizeof(wchar_t)>()));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("UTF-8 test");
    test->add(BOOST_TEST_CASE(&encode_test));
    test->add(BOOST_TEST_CASE(&decode_test));
    return test;
}
