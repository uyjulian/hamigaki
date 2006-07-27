//  pcm_play.cpp: a simple WAVE player

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/pcm_device.hpp>
#include <hamigaki/audio/wave_file.hpp>
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

        audio::wave_file_source file(argv[1]);

        std::cout << "rate = " << file.format().rate << std::endl;
        std::cout << "bits = " << file.format().bits() << std::endl;
        std::cout << "channels = " << file.format().channels << std::endl;

        io::copy(file, audio::pcm_sink(file.format(), 1024*4));

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
