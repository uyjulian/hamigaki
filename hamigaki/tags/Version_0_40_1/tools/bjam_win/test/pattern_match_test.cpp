// pattern_match_test.cpp: test cases for pattern_match

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "../impl/pattern_match.hpp"
#include <boost/test/minimal.hpp>

int test_main(int, char*[])
{
    BOOST_CHECK(pattern_match("*.c")("test.c"));
    BOOST_CHECK(!pattern_match("*.c")("test.cpp"));

    BOOST_CHECK(pattern_match("*.cpp")("test.cpp"));
    BOOST_CHECK(!pattern_match("*.cpp")("test.c"));

    BOOST_CHECK(pattern_match("a?c")("abc"));
    BOOST_CHECK(!pattern_match("a?c")("ac"));

    BOOST_CHECK(pattern_match("[a-z]")("a"));
    BOOST_CHECK(!pattern_match("[a-z]")("0"));

    BOOST_CHECK(!pattern_match("[^a-z]")("a"));
    BOOST_CHECK(pattern_match("[^a-z]")("0"));

    BOOST_CHECK(pattern_match("[]]")("]"));
    BOOST_CHECK(!pattern_match("[]]")("a"));

    BOOST_CHECK(pattern_match("[a]")("a"));
    BOOST_CHECK(!pattern_match("[a]")("b"));

    BOOST_CHECK(pattern_match("[ab]")("a"));
    BOOST_CHECK(pattern_match("[ab]")("b"));
    BOOST_CHECK(!pattern_match("[ab]")("c"));

    BOOST_CHECK(!pattern_match("\\*")("test.c"));
    BOOST_CHECK(pattern_match("\\*")("*"));

    BOOST_CHECK(pattern_match("\\[")("["));
    BOOST_CHECK(pattern_match("\\[[a]")("[a"));

    return 0;
}
