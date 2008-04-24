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

    charset::wstring ws3(L"A\u00C0\u3042\U0001D11E");
    BOOST_CHECK_EQUAL(
        charset::to_utf8(ws3), "A\xC3\x80\xE3\x81\x82\xF0\x9D\x84\x9E");

    charset::wstring ws4(L"\U0010FFFF");
    BOOST_CHECK_EQUAL(charset::to_utf8(ws4), "\xF4\x8F\xBF\xBF");

    boost::uint32_t wc5 = 0x110000;
    const wchar_t ws5[] = { static_cast<wchar_t>(wc5), 0 };
    BOOST_CHECK_THROW(charset::to_utf8(ws5), charset::invalid_ucs4);
}

void encode_test_impl(const size_tag<2>&)
{
    charset::wstring ws(L"\xD867\xDE3D");
    BOOST_CHECK_EQUAL(charset::to_utf8(ws), "\xF0\xA9\xB8\xBD");

    charset::wstring ws2(L"\xD867\xDE3D.txt");
    BOOST_CHECK_EQUAL(charset::to_utf8(ws2), "\xF0\xA9\xB8\xBD.txt");

    charset::wstring ws3(L"A\x00C0\x3042\xD834\xDD1E");
    BOOST_CHECK_EQUAL(
        charset::to_utf8(ws3), "A\xC3\x80\xE3\x81\x82\xF0\x9D\x84\x9E");

    charset::wstring ws4(L"\xDBFF\xDFFF");
    BOOST_CHECK_EQUAL(charset::to_utf8(ws4), "\xF4\x8F\xBF\xBF");

    charset::wstring ws5(L"\xD867\xFE3D");
    BOOST_CHECK_THROW(charset::to_utf8(ws5), charset::invalid_surrogate_pair);

    charset::wstring ws6(L"\xDE3D");
    BOOST_CHECK_THROW(charset::to_utf8(ws6), charset::missing_high_surrogate);

    charset::wstring ws7(L"\xD867");
    BOOST_CHECK_THROW(charset::to_utf8(ws7), charset::missing_low_surrogate);
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

    const charset::wstring ws3(L"A\u00C0\u3042\U0001D11E");
    BOOST_CHECK(charset::from_utf8(
        "A\xC3\x80\xE3\x81\x82\xF0\x9D\x84\x9E") == ws3);

    const charset::wstring ws4(L"\U0010FFFF");
    BOOST_CHECK(charset::from_utf8("\xF4\x8F\xBF\xBF") == ws4);

    BOOST_CHECK_THROW(
        charset::from_utf8("\xF4\xA0\x80\x80"), charset::invalid_ucs4);

    BOOST_CHECK_THROW(charset::from_utf8("\x80"), charset::invalid_utf8);
    BOOST_CHECK_THROW(charset::from_utf8("\xC3\xC0"), charset::invalid_utf8);
}

void decode_test_impl(const size_tag<2>&)
{
    const charset::wstring ws(L"\xD867\xDE3D");
    BOOST_CHECK(charset::from_utf8("\xF0\xA9\xB8\xBD") == ws);

    const charset::wstring ws2(L"\xD867\xDE3D.txt");
    BOOST_CHECK(charset::from_utf8("\xF0\xA9\xB8\xBD.txt") == ws2);

    const charset::wstring ws3(L"A\x00C0\x3042\xD834\xDD1E");
    BOOST_CHECK(charset::from_utf8(
        "A\xC3\x80\xE3\x81\x82\xF0\x9D\x84\x9E") == ws3);

    const charset::wstring ws4(L"\xDBFF\xDFFF");
    BOOST_CHECK(charset::from_utf8("\xF4\x8F\xBF\xBF") == ws4);

    BOOST_CHECK_THROW(
        charset::from_utf8("\xF4\xA0\x80\x80"), charset::invalid_ucs4);

    BOOST_CHECK_THROW(charset::from_utf8("\x80"), charset::invalid_utf8);
    BOOST_CHECK_THROW(charset::from_utf8("\xC3\xC0"), charset::invalid_utf8);
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
