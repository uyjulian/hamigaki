// read_link.cpp: show the target path of the symbolic link

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include <hamigaki/filesystem/operations.hpp>
#include <exception>
#include <iostream>

namespace fs_ex = hamigaki::filesystem;
namespace fs = boost::filesystem;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: read_link (path)" << std::endl;
            return 1;
        }

        fs::path ph(argv[1]);
        std::cout << fs_ex::symlink_target(ph).string() << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
