// vorbis_file_example.cpp: an example for vorbis_file<Source>

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/vorbis_file.hpp>
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
                << "Usage: vorbis_file_example (input file) (output file)"
                << std::endl;

            return 1;
        }

        audio::vorbis_file_source vf(argv[1]);
        const audio::vorbis_info& info = vf.info();
        audio::pcm_format fmt;
        fmt.type = audio::int_le16;
        fmt.channels = info.channels;
        fmt.rate = info.rate;
        io::copy(
            vf,
            audio::widen<float>(
                audio::wave_file_sink(argv[2], fmt)
            )
        );

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
