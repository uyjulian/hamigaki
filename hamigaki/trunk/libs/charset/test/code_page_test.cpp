// code_page.cpp: test case for code_page.hpp

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/charset for library home page.

#include <hamigaki/charset/code_page.hpp>
#include <boost/test/unit_test.hpp>

namespace charset = hamigaki::charset;
namespace ut = boost::unit_test;

void acp_test()
{
    charset::wstring ws(L"Hello");
    std::string s("Hello");

    BOOST_CHECK_EQUAL(charset::to_code_page(ws, 0, "_"), s);

    const charset::wstring& ws2 = charset::from_code_page(s, 0);
    BOOST_CHECK_EQUAL_COLLECTIONS(ws2.begin(), ws2.end(), ws.begin(), ws.end());

    BOOST_CHECK(
        charset::from_code_page(std::string(32, ' '), 0) ==
        charset::wstring(32, L' ')
    );
}

void sjis_test()
{
    charset::wstring ws(L"\u3053\u3093\u306B\u3061\u306F");
    std::string s("\x82\xB1\x82\xF1\x82\xC9\x82\xBF\x82\xCD");

    BOOST_CHECK_EQUAL(charset::to_code_page(ws, 932, "_"), s);

    const charset::wstring& ws2 = charset::from_code_page(s, 932);
    BOOST_CHECK_EQUAL_COLLECTIONS(ws2.begin(), ws2.end(), ws.begin(), ws.end());
}

void jis_test()
{
    charset::wstring ws(L"\u3053\u3093\u306B\u3061\u306F");
    std::string s(
        "\x1B\x24\x42\x24\x33\x24\x73\x24\x4B\x24\x41\x24\x4F\x1B\x28\x42");

    // The option def_char doesn't work on Windows
    BOOST_CHECK_EQUAL(charset::to_code_page(ws, 50220), s);

    const charset::wstring& ws2 = charset::from_code_page(s, 50220);
    BOOST_CHECK_EQUAL_COLLECTIONS(ws2.begin(), ws2.end(), ws.begin(), ws.end());
}

void eucjp_test()
{
    charset::wstring ws(L"\u3053\u3093\u306B\u3061\u306F");
    std::string s("\xA4\xB3\xA4\xF3\xA4\xCB\xA4\xC1\xA4\xCF");

    BOOST_CHECK_EQUAL(charset::to_code_page(ws, 20932, "_"), s);

    const charset::wstring& ws2 = charset::from_code_page(s, 20932);
    BOOST_CHECK_EQUAL_COLLECTIONS(ws2.begin(), ws2.end(), ws.begin(), ws.end());
}

void gbk_test()
{
    charset::wstring ws(L"\u4F60\u597D");
    std::string s("\xC4\xE3\xBA\xC3");

    BOOST_CHECK_EQUAL(charset::to_code_page(ws, 936, "_"), s);

    const charset::wstring& ws2 = charset::from_code_page(s, 936);
    BOOST_CHECK_EQUAL_COLLECTIONS(ws2.begin(), ws2.end(), ws.begin(), ws.end());
}

void uhc_test()
{
    charset::wstring ws(L"\uC548\uB155\uD558\uC2ED\uB2C8\uAE4C");
    std::string s("\xBE\xC8\xB3\xE7\xC7\xCF\xBD\xCA\xB4\xCF\xB1\xEE");

    BOOST_CHECK_EQUAL(charset::to_code_page(ws, 949, "_"), s);

    const charset::wstring& ws2 = charset::from_code_page(s, 949);
    BOOST_CHECK_EQUAL_COLLECTIONS(ws2.begin(), ws2.end(), ws.begin(), ws.end());
}

template<std::size_t N>
struct size_tag {};

void surrogate_test_impl(const size_tag<4>&)
{
}

void surrogate_test_impl(const size_tag<2>&)
{
    charset::wstring ws(L"\xD867\xDE3D");
    BOOST_CHECK_EQUAL(charset::to_code_page(ws, 932, "_"), "__");
    BOOST_CHECK_EQUAL(
        charset::to_code_page(ws, 65001), "\xF0\xA9\xB8\xBD");

    charset::wstring ws2(L"\xD867\xDE3D.txt");
    BOOST_CHECK_EQUAL(charset::to_code_page(ws2, 932, "_"), "__.txt");
    BOOST_CHECK_EQUAL(
        charset::to_code_page(ws2, 65001), "\xF0\xA9\xB8\xBD.txt");
}

void surrogate_test()
{
    surrogate_test_impl((size_tag<sizeof(wchar_t)>()));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("code page test");
    test->add(BOOST_TEST_CASE(&acp_test));
    test->add(BOOST_TEST_CASE(&sjis_test));
    test->add(BOOST_TEST_CASE(&jis_test));
    test->add(BOOST_TEST_CASE(&eucjp_test));
    test->add(BOOST_TEST_CASE(&gbk_test));
    test->add(BOOST_TEST_CASE(&uhc_test));
    test->add(BOOST_TEST_CASE(&surrogate_test));
    return test;
}
