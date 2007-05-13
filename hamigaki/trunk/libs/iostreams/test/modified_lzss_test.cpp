// modified_lzss_test.cpp: test case for modified LZSS

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/filter/modified_lzss.hpp>
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
const io_ex::bit_flow flow = io_ex::left_to_right;
const hamigaki::endianness endian = hamigaki::native;
const std::size_t offset_bits = 16;
const std::size_t length_bits = 8;

std::string modified_lzss_compress(const std::string& src)
{
    const char* start = src.c_str();

    std::string dst;
    io::copy(
        io::array_source(start, start + src.size()),
        io::compose(
            io_ex::modified_lzss_compressor<
                flow,endian,offset_bits,length_bits>(window_bits),
            io::back_inserter(dst)
        )
    );
    return dst;
}

std::string modified_lzss_decompress(const std::string& src, std::size_t size)
{
    const char* start = src.c_str();

    std::string dst;
    io::copy(
        io::compose(
            io_ex::modified_lzss_decompressor<
                flow,endian,offset_bits,length_bits>(window_bits),
            io::array_source(start, start + src.size())
        ),
        io::back_inserter(dst)
    );
    return dst;
}

bool modified_lzss_test_aux(const std::string& s)
{
    return modified_lzss_decompress(modified_lzss_compress(s), s.size()) == s;
}

void modified_lzss_test()
{
    BOOST_CHECK(modified_lzss_test_aux(""));
    BOOST_CHECK(modified_lzss_test_aux("a"));
    BOOST_CHECK(modified_lzss_test_aux("aa"));
    BOOST_CHECK(modified_lzss_test_aux("aaa"));
    BOOST_CHECK(modified_lzss_test_aux("aaaa"));
    BOOST_CHECK(modified_lzss_test_aux("aaaaa"));
    BOOST_CHECK(modified_lzss_test_aux("ababababa"));
    BOOST_CHECK(modified_lzss_test_aux("ababababbabababa"));

    BOOST_CHECK(modified_lzss_test_aux(std::string(4096+1, 'a')));
    BOOST_CHECK(modified_lzss_test_aux(std::string(4096*2+1, 'a')));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("modified LZSS test");
    test->add(BOOST_TEST_CASE(&modified_lzss_test));
    return test;
}
