// pcm_play.cpp: a simple WAVE player

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/pcm_device.hpp>
#include <hamigaki/audio/wave_file.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <boost/iostreams/copy.hpp>
#include <exception>
#include <iostream>

namespace audio = hamigaki::audio;
namespace io = boost::iostreams;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr
                << "Usage: pcm_play (input file)"
                << std::endl;

            return 1;
        }

        audio::wave_file_source wav(argv[1]);

        audio::pcm_format fmt = wav.format();
        std::cout << "rate = " << fmt.rate << std::endl;
        std::cout << "bits = " << fmt.bits() << std::endl;
        std::cout << "channels = " << fmt.channels << std::endl;

        io::copy(wav, audio::pcm_sink(fmt));

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
