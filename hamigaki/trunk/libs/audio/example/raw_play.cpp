//  raw_play.cpp: pcm raw data (22kHz,16bit,Mono) player

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hamigaki/audio/pcm_device.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/copy.hpp>
#include <exception>
#include <iostream>

namespace io = boost::iostreams;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr
                << "Usage: raw_play (input file)"
                << std::endl;

            return 1;
        }

        hamigaki::audio::pcm_format fmt;
        fmt.type = hamigaki::audio::int_le16;
        fmt.channels = 1;
        fmt.rate = 22050;

        io::copy(
            io::file_source((std::string(argv[1])), BOOST_IOS::binary),
            hamigaki::audio::pcm_sink(fmt, 1024));

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
