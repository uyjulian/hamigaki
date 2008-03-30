// config_info.cpp: dummy test for "Compiler Status"

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/config.hpp>
#include <boost/version.hpp>
#include <iostream>

int main()
{
    std::cout << BOOST_COMPILER << "\n";
    std::cout << "Detected Platform: " << BOOST_PLATFORM << std::endl;
    std::cout << "Boost version " << BOOST_STRINGIZE(BOOST_VERSION) <<std::endl;

    return 0;
}
