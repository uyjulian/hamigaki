// aiff_play.cpp: a simple AIFF player

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/aiff_file.hpp>
#include <hamigaki/audio/pcm_device.hpp>
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
                << "Usage: aiff_play (input file)"
                << std::endl;

            return 1;
        }

        audio::aiff_file_source file(argv[1]);

        std::cout << "rate = " << file.format().rate << std::endl;
        std::cout << "bits = " << file.format().bits() << std::endl;
        std::cout << "channels = " << file.format().channels << std::endl;

        audio::pcm_format fmt = file.format();
        if (fmt.type == audio::int8)
            fmt.type = audio::uint8;
        else
            fmt.type = audio::int_le16;

        io::copy(
            audio::widen<boost::int_t<16> >(file),
            audio::widen<boost::int_t<16> >(audio::pcm_sink(fmt))
        );

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
