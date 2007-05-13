// lazy_restrict_test.cpp: test case for lazy_restrict

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/lazy_restrict.hpp>
#include <boost/iostreams/detail/adapter/direct_adapter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

template<std::size_t N, class Source>
void check_array(char (&data)[N],
    io::stream_offset offset, io::stream_offset len, Source src)
{
    typedef std::char_traits<char> traits_type;

    for (io::stream_offset i = 0; i < len; ++i)
    {
        BOOST_CHECK_EQUAL(
            io::get(src),
            traits_type::to_int_type(
                data[static_cast<std::ptrdiff_t>(i+offset)]
            )
        );
    }

    BOOST_CHECK_EQUAL(io::get(src), traits_type::eof());
}

void lazy_restrict_test()
{
    char data[100];
    std::copy(
        boost::counting_iterator<char>(0),
        boost::counting_iterator<char>(100),
        &data[0]);

#if BOOST_WORKAROUND(BOOST_MSVC, == 1310)
    io::detail::io_mode_id<io::array_source>::value;
#endif

    // TODO: lazy_restrict requires indirect
    typedef io::detail::direct_adapter<io::array_source> source_type;
    source_type src(io::array_source(&data[0], &data[0] + sizeof(data)));

    check_array(
        data, 10, 40,
        io_ex::lazy_restriction<source_type>(src, 10, 40)
    );
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("lazy_restrict test");
    test->add(BOOST_TEST_CASE(&lazy_restrict_test));
    return test;
}
