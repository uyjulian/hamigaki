// background_copy_test.cpp: test case for background_copy

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/device/zero.hpp>
#include <hamigaki/iostreams/background_copy.hpp>
#include <hamigaki/thread/utc_time.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/test/unit_test.hpp>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

void background_copy_test()
{
    const unsigned long test_size = 32ul * 1024ul * 1024ul;

    {
        io_ex::background_copy bg_copy(
            io::restrict(io_ex::zero_source(), 0, test_size),
            io::null_sink());

        hamigaki::thread::utc_time t;
        while (!bg_copy.done())
        {
            t += hamigaki::thread::milliseconds(100);
            boost::thread::sleep(t);
        }
    }
    {
        io_ex::background_copy bg_copy(
            io::restrict(io_ex::zero_source(), 0, test_size),
            io::null_sink());

        bg_copy.stop();
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("background_copy test");
    test->add(BOOST_TEST_CASE(&background_copy_test));
    return test;
}
