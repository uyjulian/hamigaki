// base64_test.cpp: test case for Base64 encoder/decoder

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/filter/base64.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp> 
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/test/unit_test.hpp>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

std::string base64_encode(const std::string& src)
{
    const char* start = src.c_str();

    std::string dst;
    io::copy(
        io::array_source(start, start + src.size()),
        io::compose(io_ex::base64_encoder(), io::back_inserter(dst))
    );
    return dst;
}

std::string base64_decode(const std::string& src)
{
    const char* start = src.c_str();

    std::string dst;
    io::copy(
        io::compose(
            io_ex::base64_decoder(),
            io::array_source(start, start + src.size())
        ),
        io::back_inserter(dst)
    );
    return dst;
}

void base64_test()
{
    BOOST_CHECK_EQUAL(
        base64_encode("\x14\xfb\x9c\x03\xd9\x7f"), std::string("FPucA9l/"));
    BOOST_CHECK_EQUAL(
        base64_encode("\x14\xfb\x9c\x03\xd9\x7e"), std::string("FPucA9l+"));
    BOOST_CHECK_EQUAL(
        base64_encode("\x14\xfb\x9c\x03\xd9"), std::string("FPucA9k="));
    BOOST_CHECK_EQUAL(
        base64_encode("\x14\xfb\x9c\x03"), std::string("FPucAw=="));

    BOOST_CHECK_EQUAL(
        base64_decode("FPucA9l/"), std::string("\x14\xfb\x9c\x03\xd9\x7f"));
    BOOST_CHECK_EQUAL(
        base64_decode("FPucA9l+"), std::string("\x14\xfb\x9c\x03\xd9\x7e"));
    BOOST_CHECK_EQUAL(
        base64_decode("FPucA9k="), std::string("\x14\xfb\x9c\x03\xd9"));
    BOOST_CHECK_EQUAL(
        base64_decode("FPucAw=="), std::string("\x14\xfb\x9c\x03"));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("Base64 test");
    test->add(BOOST_TEST_CASE(&base64_test));
    return test;
}
