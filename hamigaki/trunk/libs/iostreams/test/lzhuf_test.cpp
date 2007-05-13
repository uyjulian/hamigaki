// lzhuf_test.cpp: test case for LZHUF compressor/decompressor

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/filter/lzhuf.hpp>
#include <hamigaki/iostreams/tiny_restrict.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/test/unit_test.hpp>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

const std::size_t window_bits = 13;

std::string lzhuf_compress(const std::string& src)
{
    const char* start = src.c_str();

    std::string dst;
    io::copy(
        io::array_source(start, start + src.size()),
        io::compose(
            io_ex::lzhuf_compressor(window_bits),
            io::back_inserter(dst)
        )
    );
    return dst;
}

std::string lzhuf_decompress(const std::string& src, std::size_t size)
{
    const char* start = src.c_str();

    std::string dst;
    io::copy(
        io_ex::tiny_restrict(
            io::compose(
                io_ex::lzhuf_decompressor(window_bits),
                io::array_source(start, start + src.size())
            ),
            size
        ),
        io::back_inserter(dst)
    );
    return dst;
}

bool lzhuf_test_aux(const std::string& s)
{
    return lzhuf_decompress(lzhuf_compress(s), s.size()) == s;
}

void lzhuf_test()
{
    BOOST_CHECK(lzhuf_test_aux(""));
    BOOST_CHECK(lzhuf_test_aux("a"));
    BOOST_CHECK(lzhuf_test_aux("aa"));
    BOOST_CHECK(lzhuf_test_aux("aaa"));
    BOOST_CHECK(lzhuf_test_aux("aaaa"));
    BOOST_CHECK(lzhuf_test_aux("aaaaa"));
    BOOST_CHECK(lzhuf_test_aux("ababababa"));
    BOOST_CHECK(lzhuf_test_aux("ababababbabababa"));

    BOOST_CHECK(lzhuf_test_aux(std::string(4096+1, 'a')));
    BOOST_CHECK(lzhuf_test_aux(std::string(4096*2+1, 'a')));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("LZHUF test");
    test->add(BOOST_TEST_CASE(&lzhuf_test));
    return test;
}
