// uuid_test.cpp: test case for uuid

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/uuid for library home page.

#include <hamigaki/uuid.hpp>
#include <boost/test/unit_test.hpp>

namespace ut = boost::unit_test;

void uuid_test()
{
    BOOST_CHECK_EQUAL(
        hamigaki::uuid().to_string(),
        std::string("00000000-0000-0000-0000-000000000000")
    );

    BOOST_CHECK_EQUAL(
        hamigaki::uuid("f81d4fae-7dec-11d0-a765-00a0c91e6bf6").to_string(),
        std::string("f81d4fae-7dec-11d0-a765-00a0c91e6bf6")
    );

    BOOST_CHECK_EQUAL(
        hamigaki::uuid(
            "{F81D4FAE-7DEC-11D0-A765-00A0C91E6BF6}").to_guid_string(),
        std::string("{F81D4FAE-7DEC-11D0-A765-00A0C91E6BF6}")
    );

#if !defined(BOOST_NO_STD_WSTRING)
    BOOST_CHECK(
        hamigaki::uuid().to_wstring() ==
        L"00000000-0000-0000-0000-000000000000"
    );

    BOOST_CHECK(
        hamigaki::uuid(L"f81d4fae-7dec-11d0-a765-00a0c91e6bf6").to_wstring() ==
        L"f81d4fae-7dec-11d0-a765-00a0c91e6bf6"
    );

    BOOST_CHECK(
        hamigaki::uuid(
            L"{F81D4FAE-7DEC-11D0-A765-00A0C91E6BF6}").to_guid_wstring() ==
        L"{F81D4FAE-7DEC-11D0-A765-00A0C91E6BF6}"
    );
#endif
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("uuid test");
    test->add(BOOST_TEST_CASE(&uuid_test));
    return test;
}
