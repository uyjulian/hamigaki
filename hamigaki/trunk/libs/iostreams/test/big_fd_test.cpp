// big_fd_test.cpp: test case for a big file descriptor

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/device/file_descriptor.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <cstdio>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

void big_fd_test()
{
    const std::string filename("big_fd_test.dat");

    const boost::uint64_t seed = 0x0123456789ABCDEFull;
    boost::rand48 rand_gen(seed);

    boost::variate_generator<
        boost::rand48&,
        boost::uniform_int<unsigned char>
    > rand(rand_gen, boost::uniform_int<unsigned char>(0, 0xFF));

    char data[4096];

    const std::streamsize size = static_cast<std::streamsize>(sizeof(data));

    io_ex::file_descriptor_sink sink(filename);

    for (int i = 0; i < 0x100000+1; ++i)
    {
        std::generate_n(data, sizeof(data), rand);
        BOOST_CHECK_EQUAL(sink.write(data, size), size);
    }

    BOOST_CHECK_EQUAL(
        io::position_to_offset(sink.seek(0, BOOST_IOS::cur)), 0x100001000ull);
    BOOST_CHECK_EQUAL(
        io::position_to_offset(sink.seek(-1, BOOST_IOS::cur)), 0x100000FFFull);

    sink.close();

    rand_gen.seed(seed);

    io_ex::file_descriptor_source src(filename);

    char data2[4096];

    for (int i = 0; i < 0x100000+1; ++i)
    {
        BOOST_CHECK_EQUAL(src.read(data2, size), size);

        std::generate_n(data, sizeof(data), rand);
        BOOST_CHECK_EQUAL_COLLECTIONS(data, data+size, data2, data2+size);
    }

    BOOST_CHECK_EQUAL(
        io::position_to_offset(src.seek(0, BOOST_IOS::cur)), 0x100001000ull);
    BOOST_CHECK_EQUAL(
        io::position_to_offset(src.seek(-1, BOOST_IOS::cur)), 0x100000FFFull);

    src.close();

    std::remove(filename.c_str());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("big file descriptor test");
    test->add(BOOST_TEST_CASE(&big_fd_test));
    return test;
}
