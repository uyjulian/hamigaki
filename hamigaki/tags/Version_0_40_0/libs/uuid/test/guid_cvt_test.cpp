// guid_cvt_test.cpp: a test for GUID <-> uuid conversion

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/uuid for library home page.

#include <hamigaki/uuid.hpp>
#include <boost/test/unit_test.hpp>
#include <windows.h>

#if !defined(__GNUC__)
    #pragma comment(lib, "ole32.lib")
#endif

namespace ut = boost::unit_test;

void guid_test()
{
    wchar_t buf[] = L"{F81D4FAE-7DEC-11D0-A765-00A0C91E6BF6}";
    ::GUID id;
    ::CLSIDFromString(buf, &id);
    BOOST_CHECK_EQUAL(
        hamigaki::uuid(id).to_guid_string(),
        std::string("{F81D4FAE-7DEC-11D0-A765-00A0C91E6BF6}")
    );

    ::GUID id2;
    hamigaki::uuid(id).copy(id2);
    BOOST_CHECK(std::memcmp(&id, &id2, sizeof(::GUID)) == 0);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("GUID/uuid conversion test");
    test->add(BOOST_TEST_CASE(&guid_test));
    return test;
}
