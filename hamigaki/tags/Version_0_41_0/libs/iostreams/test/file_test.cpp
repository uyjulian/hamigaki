// file_test.cpp: test case for file device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/device/file.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <cstdio>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

void file_test()
{
    const std::string filename("file_test.dat");

    char data[100];
    std::copy(
        boost::counting_iterator<char>(0),
        boost::counting_iterator<char>(100),
        &data[0]);

    const std::streamsize size = static_cast<std::streamsize>(sizeof(data));

    io_ex::file_sink sink(filename, BOOST_IOS::binary);
    BOOST_CHECK(sink.is_open());

    BOOST_CHECK_EQUAL(sink.write(data, size), size);
    BOOST_CHECK_EQUAL(
        io::position_to_offset(sink.seek(0, BOOST_IOS::cur)), 100);
    BOOST_CHECK_EQUAL(io::position_to_offset(sink.seek(0, BOOST_IOS::beg)), 0);

    sink.close();
    BOOST_CHECK(!sink.is_open());

    io_ex::file_source src(filename, BOOST_IOS::binary);
    BOOST_CHECK(src.is_open());

    char data2[100];
    BOOST_CHECK_EQUAL(src.read(data2, size), size);

    BOOST_CHECK_EQUAL_COLLECTIONS(data, data+size, data2, data2+size);

    BOOST_CHECK_EQUAL(
        io::position_to_offset(src.seek(0, BOOST_IOS::cur)), 100);
    BOOST_CHECK_EQUAL(io::position_to_offset(src.seek(0, BOOST_IOS::beg)), 0);

    src.close();
    BOOST_CHECK(!src.is_open());

    std::remove(filename.c_str());
}

void io_test()
{
    const std::string filename("file_test.dat");

    char data[100];
    std::copy(
        boost::counting_iterator<char>(0),
        boost::counting_iterator<char>(100),
        &data[0]);

    const std::streamsize size = static_cast<std::streamsize>(sizeof(data));

    io_ex::file dev(filename, BOOST_IOS::trunc|BOOST_IOS::binary);
    BOOST_CHECK(dev.is_open());

    BOOST_CHECK_EQUAL(dev.write(data, size), size);
    BOOST_CHECK_EQUAL(
        io::position_to_offset(dev.seek(0, BOOST_IOS::cur)), 100);
    BOOST_CHECK_EQUAL(io::position_to_offset(dev.seek(0, BOOST_IOS::beg)), 0);

    char data2[100];
    BOOST_CHECK_EQUAL(dev.read(data2, size), size);

    BOOST_CHECK_EQUAL_COLLECTIONS(data, data+size, data2, data2+size);

    BOOST_CHECK_EQUAL(
        io::position_to_offset(dev.seek(0, BOOST_IOS::cur)), 100);
    BOOST_CHECK_EQUAL(io::position_to_offset(dev.seek(0, BOOST_IOS::beg)), 0);

    dev.close();

    BOOST_CHECK(!dev.is_open());

    std::remove(filename.c_str());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("file test");
    test->add(BOOST_TEST_CASE(&file_test));
    test->add(BOOST_TEST_CASE(&io_test));
    return test;
}
