//  glob.cpp: glob for bjam

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#include "glob.hpp"
#include "impl/pattern_match.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/operations.hpp>

namespace algo = boost::algorithm;
namespace fs = boost::filesystem;

std::vector<std::string> glob(
    const boost::filesystem::path& work,
    const std::string& dir, const std::string& pattern)
{
    std::vector<std::string> result;

    fs::path ph(dir, fs::native);
    if (!ph.has_root_directory())
        ph = work / ph;
    else if (!ph.has_root_name())
        ph = work.root_name() / ph;

    // Note:
    // ".."             -> "..\\"
    // "../"            -> "..\\"
    // "c:/"            -> "c:/"
    // "c:\\"           -> "c:\\"
    // "c:/Temp"        -> "c:\\Temp\\"
    // "c:/Temp\\"      -> "c:\\Temp\\"
    std::string prefix = dir;
    if ((prefix.size() != 3) || (prefix[1] != ':'))
    {
        algo::replace_all(prefix, "/", "\\");
        if (!prefix.empty() && (*prefix.rbegin() != '\\'))
            prefix += '\\';
    }

    fs::directory_iterator it(ph);
    fs::directory_iterator end;

    pattern_match ptn_match(algo::to_lower_copy(pattern));
    for (; it != end; ++it)
    {
        std::string leaf = it->leaf();
        if (ptn_match(algo::to_lower_copy(leaf)))
            result.push_back(prefix + leaf);
    }

    return result;
}
