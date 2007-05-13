// path_glob_test.cpp: a test driver for path::glob()

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "../impl/path.hpp"
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
        if ((argc != 3) && (argc != 4))
        {
            std::cerr
                << "Usage: path_glob_test (dir) (pattern) (exclude-pattern)"
                << std::endl;
            return 1;
        }

        fs::path work = fs::current_path();

        std::vector<std::string> dirs;
        dirs.push_back(argv[1]);

        std::vector<std::string> patterns;
        patterns.push_back(argv[2]);

        std::vector<std::string> ex_patterns;
        if (argc == 4)
            ex_patterns.push_back(argv[3]);

        const std::vector<std::string>& vs =
            path::glob(work, dirs, patterns, ex_patterns);

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
