// asio_ogg_play.cpp: Ogg/Vorbis Player for ASIO

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/asio/drivers.hpp>
#include <hamigaki/audio/asio.hpp>
#include <hamigaki/audio/vorbis_file.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <hamigaki/iostreams/demultiplexer.hpp>
#include <boost/iostreams/copy.hpp>
#include <exception>
#include <iostream>

#include <hamigaki/detail/windows/com_library.hpp>
#include <windows.h>

namespace audio = hamigaki::audio;
namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
using namespace hamigaki::detail::windows;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr
                << "Usage: asio_ogg_play (input file)"
                << std::endl;

            return 1;
        }

        ::SetThreadPriority(
            ::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        com_library using_com;

        audio::vorbis_file_source vf(argv[1]);
        const audio::vorbis_info& info = vf.info();

        audio::asio_device asio(audio::asio::driver_list().at(0).clsid);
        asio.rate(info.rate);

        asio.create_buffers(0, info.channels);

        io_ex::basic_demultiplexer<float> demuxer;
        for (int i = 0; i < info.channels; ++i)
            demuxer.push(audio::widen<float>(asio.get_sink(i)));

        io::copy(vf, demuxer, info.channels);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
