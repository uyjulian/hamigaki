//  base64_encoder_example.cpp: an example for base64_encoder

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hamigaki/iostreams/base64_encoder.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp> 
#include <boost/iostreams/copy.hpp>
#include <exception>
#include <iostream>
#include <string>

namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;

int main()
{
    try
    {
        char src[] = "test";
        std::string dst;
        io::copy(
            io::array_source(src, src+4),
            io_ex::base64_encoded(io::back_inserter(dst)));
        std::cout << dst << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
