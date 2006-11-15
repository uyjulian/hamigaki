//  link.cpp: create soft/hard links

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include <hamigaki/filesystem/operations.hpp>
#include <exception>
#include <iostream>

namespace fs_ex = hamigaki::filesystem;
namespace fs = boost::filesystem;

int main(int argc, char* argv[])
{
    try
    {
        if (!(
            (argc == 3) ||
            ((argc == 4) && (std::strcmp(argv[1], "-s") == 0))
        ))
        {
            std::cerr << "Usage: link [-s] (target) (path)" << std::endl;
            return 1;
        }

        if (argc == 3)
        {
            fs::path target(argv[1], fs::native);
            fs::path ph(argv[2], fs::native);
            fs_ex::create_hard_link(target, ph);
        }
        else
        {
            fs::path target(argv[2], fs::native);
            fs::path ph(argv[3], fs::native);
            fs_ex::create_symlink(target, ph);
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
