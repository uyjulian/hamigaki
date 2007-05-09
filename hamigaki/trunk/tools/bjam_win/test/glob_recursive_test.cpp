//  glob_recursive_test.cpp: a test driver for glob_recursive()

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#include "../impl/glob.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/version.hpp>
#include <exception>
#include <iostream>
#include <iterator>

namespace fs = boost::filesystem;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr
                << "Usage: glob_recursive_test (pattern)" << std::endl;
            return 1;
        }

        fs::path work = fs::current_path();

        const std::vector<std::string>& vs = glob_recursive(work, argv[1]);

        std::copy(
            vs.begin(), vs.end(),
            std::ostream_iterator<std::string>(std::cout, "\n")
        );

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
