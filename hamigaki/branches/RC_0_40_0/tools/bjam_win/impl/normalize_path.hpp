// normalize_path.hpp: NORMALIZE_PATH rule

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_NORMALIZE_PATH_HPP
#define IMPL_NORMALIZE_PATH_HPP

#include <string>
#include <vector>

inline std::string normalize_path(const std::vector<std::string>& parts)
{
    bool rooted = !parts.empty() && !parts[0].empty() && (parts[0][0] == '/');

    std::vector<std::string> tmp;
    for (std::size_t i = 0, size = parts.size(); i < size; ++i)
    {
        const std::string& part = parts[i];
        if (part.empty() || (part == "."))
            continue;
        else if (part == "..")
        {
            if (tmp.empty())
                tmp.push_back("..");
            else
                tmp.pop_back();
        }
        else if (part[0] == '/')
            tmp.push_back(part.substr(1));
        else
            tmp.push_back(part);
    }

    std::string result;
    if (rooted)
        result += '/';
    for (std::size_t i = 0, size = tmp.size(); i < size; ++i)
    {
        if (i != 0)
            result += '/';
        result += tmp[i];
    }

    if (result.empty())
        result = ".";

    return result;
}

#endif // IMPL_NORMALIZE_PATH_HPP
