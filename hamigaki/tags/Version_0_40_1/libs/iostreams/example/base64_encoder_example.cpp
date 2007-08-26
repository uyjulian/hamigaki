// base64_encoder_example.cpp: an example for base64_encoder

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/filter/base64.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp> 
#include <boost/iostreams/compose.hpp>
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
            io::compose(io_ex::base64_encoder(), io::back_inserter(dst)));
        std::cout << dst << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
