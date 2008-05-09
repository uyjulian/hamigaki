// pa_capture.cpp: a WAVE recorder by using PulseAudio

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/pulse_audio.hpp>
#include <hamigaki/audio/wave_file.hpp>
#include <hamigaki/iostreams/background_copy.hpp>
#include <exception>
#include <iostream>
#include <limits>

namespace audio = hamigaki::audio;
namespace io_ex = hamigaki::iostreams;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr
                << "Usage: pa_capture (output file)"
                << std::endl;

            return 1;
        }

        audio::pcm_format fmt;
        fmt.type = audio::uint8;
        fmt.channels = 1;
        fmt.rate = 22050;

        audio::wave_file_sink file(argv[1], fmt);
        io_ex::background_copy bg_copy(
            audio::pulse_audio_source("pa_capture", "source", fmt),
            file
        );

        std::cout << "Press ENTER key to stop recording..." << std::endl;
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

        bg_copy.stop();
        file.close();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
