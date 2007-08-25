// repeat_test.cpp: test case for repeat

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/repeat.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

template<std::size_t N, class Source>
void check_array(char (&data)[N], int count, Source src)
{
    typedef std::char_traits<char> traits_type;

    for (int j = 0; j < count; ++j)
    {
        for (std::size_t i = 0; i < N; ++i)
            BOOST_CHECK_EQUAL(io::get(src), traits_type::to_int_type(data[i]));
    }

    BOOST_CHECK_EQUAL(io::get(src), traits_type::eof());
}

void repeat_test()
{
    char data[100];
    std::copy(
        boost::counting_iterator<char>(0),
        boost::counting_iterator<char>(100),
        &data[0]);

#if BOOST_WORKAROUND(BOOST_MSVC, == 1310)
    io::detail::io_mode_id<io::array_source>::value;
#endif
    check_array(
        data, 3,
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
        io_ex::repeat(io::array_source(&data[0], &data[0]+sizeof(data)), 3)
#else
        io_ex::repeat(io::array_source(data), 3)
#endif
    );
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("repeat test");
    test->add(BOOST_TEST_CASE(&repeat_test));
    return test;
}
