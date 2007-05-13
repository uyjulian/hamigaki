// asio_example.cpp: a simple ASIO example

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/asio/drivers.hpp>
#include <hamigaki/audio/asio.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <hamigaki/iostreams/background_copy.hpp>
#include <hamigaki/iostreams/demultiplexer.hpp>
#include <hamigaki/iostreams/multiplexer.hpp>
#include <exception>
#include <iostream>

#include <hamigaki/detail/windows/com_library.hpp>
#include <windows.h>

namespace audio = hamigaki::audio;
namespace io_ex = hamigaki::iostreams;
using namespace hamigaki::detail::windows;

int main()
{
    try
    {
        ::SetThreadPriority(
            ::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        com_library using_com;

        audio::asio_device asio(audio::asio::driver_list().at(0).clsid);
        asio.rate(44100.0);

        asio.create_buffers(2, 2);

        io_ex::basic_multiplexer<float> muxer;
        muxer.push(audio::widen<float>(asio.get_source(0)));
        muxer.push(audio::widen<float>(asio.get_source(1)));

        io_ex::basic_demultiplexer<float> demuxer;
        demuxer.push(audio::widen<float>(asio.get_sink(0)));
        demuxer.push(audio::widen<float>(asio.get_sink(1)));

        io_ex::background_copy bg_copy(
            muxer,
            demuxer,
            asio.buffer_size()
        );

        std::cout << "Press ENTER key to stop recording" << std::endl;
        std::cin.ignore(256, '\n');

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
