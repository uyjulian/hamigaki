// vorbis_encoder_example.cpp: an example for vorbis_encoder

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/vorbis_encoder.hpp>
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
        if (argc != 3)
        {
            std::cerr
                << "Usage: vorbis_encoder_example (input file) (output file)"
                << std::endl;

            return 1;
        }

        audio::wave_file_source file(argv[1]);
        const audio::pcm_format& fmt = file.format();
        io::copy(
            audio::widen<float>(file),
            audio::vorbis_file_sink(argv[2], fmt.channels, fmt.rate)
        );

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
