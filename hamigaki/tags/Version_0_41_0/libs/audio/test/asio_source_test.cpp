// asio_source_test.cpp: test case for asio_source

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/asio/drivers.hpp>
#include <hamigaki/audio/asio.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/copy.hpp>
#include <hamigaki/iostreams/tiny_restrict.hpp>
#include <boost/test/unit_test.hpp>
#include "detail/utility.hpp"

#include <hamigaki/detail/windows/com_library.hpp>

namespace audio = hamigaki::audio;
namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

using namespace hamigaki::audio::test;

void asio_source_test()
{
    hamigaki::detail::windows::com_library using_com;

    audio::asio_device asio(audio::asio::driver_list().at(0).clsid);

    asio.create_buffers(1, 0);

    io::copy(
        io_ex::tiny_restrict(
            asio.get_source(0),
            calc_samples_per_note(static_cast<unsigned>(asio.rate()),150)
        ),
        io::null_sink()
    );
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("asio_source test");
    test->add(BOOST_TEST_CASE(&asio_source_test));
    return test;
}
