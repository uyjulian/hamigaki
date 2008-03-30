// tmp_file_test.cpp: test case for tmp_file

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/device/tmp_file.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

void tmp_file_test()
{
    char data[100];
    std::copy(
        boost::counting_iterator<char>(0),
        boost::counting_iterator<char>(100),
        &data[0]);

    const std::streamsize size = static_cast<std::streamsize>(sizeof(data));

    io_ex::tmp_file tmp;

    io_ex::tmp_file dummy[10];
    dummy[0].close();

    BOOST_CHECK_EQUAL(tmp.write(data, size), size);
    BOOST_CHECK_EQUAL(io::position_to_offset(tmp.seek(0, BOOST_IOS::beg)), 0);

    char data2[100];
    BOOST_CHECK_EQUAL(tmp.read(data2, size), size);

    BOOST_CHECK_EQUAL_COLLECTIONS(data, data+size, data2, data2+size);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("tmp_file test");
    test->add(BOOST_TEST_CASE(&tmp_file_test));
    return test;
}
