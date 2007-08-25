// background_copy_example.cpp: an example for background_copy

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/device/zero.hpp>
#include <hamigaki/iostreams/background_copy.hpp>
#include <hamigaki/thread/utc_time.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/progress.hpp>
#include <exception>
#include <iostream>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;

int main()
{
    try
    {
        const unsigned long test_size = 32ul * 1024ul * 1024ul;
        io_ex::background_copy bg_copy(
            io::restrict(io_ex::zero_source(), 0, test_size),
            io::file_sink("zero.dat", BOOST_IOS::binary));

        boost::progress_display progress(test_size);
        hamigaki::thread::utc_time t;
        while (!bg_copy.done())
        {
            progress += static_cast<unsigned long>(
                bg_copy.total() - progress.count());

            t += hamigaki::thread::milliseconds(100);
            boost::thread::sleep(t);
        }
        progress += static_cast<unsigned long>(
            progress.expected_count() - progress.count());

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
