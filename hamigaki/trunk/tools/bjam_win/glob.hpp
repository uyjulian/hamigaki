//  glob.hpp: glob for bjam

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GLOB_HPP
#define GLOB_HPP

#include <boost/filesystem/path.hpp>
#include <string>
#include <vector>

std::vector<std::string> glob(
    const boost::filesystem::path& work,
    const std::string& dir, const std::string& pattern);

#endif // GLOB_HPP
