// glob.hpp: glob for bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_GLOB_HPP
#define IMPL_GLOB_HPP

#include "./pattern_match.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/version.hpp>
#include <vector>

inline std::string path_join_nt(const std::string& ph1, const std::string& ph2)
{
    if (!ph1.empty() && (*ph1.rbegin() != '\\'))
        return ph1 + '\\' + ph2;
    else
        return ph1 + ph2;
}

inline bool contains_wildcard(const std::string& s)
{
    return s.find_first_of("?*[]") != std::string::npos;
}

inline std::vector<std::string> glob_impl(
    const boost::filesystem::path& work,
    const std::string& dir, const std::string& pattern, bool dir_only)
{
    namespace algo = boost::algorithm;
    namespace fs = boost::filesystem;

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
        if (dir_only && !is_directory(it->status()))
            continue;
        std::string leaf = it->path().leaf();
        if (ptn_match(algo::to_lower_copy(leaf)))
            result.push_back(prefix + leaf);
    }

    return result;
}

inline std::vector<std::string> glob(
    const boost::filesystem::path& work,
    const std::string& dir, const std::string& pattern)
{
    return ::glob_impl(work, dir, pattern, false);
}

inline std::vector<std::string>
glob_recursive(
    const boost::filesystem::path& work,
    const std::string& dir, const std::string& pattern)
{
    namespace fs = boost::filesystem;

    std::string::size_type slash = pattern.find_first_of("/\\");
    if (slash == std::string::npos)
    {
        if (::contains_wildcard(pattern))
            return ::glob_impl(work, dir, pattern, false);
        else
        {
            const std::string& ph = ::path_join_nt(dir, pattern);

            std::vector<std::string> tmp;
            if (fs::exists(fs::complete(fs::path(ph, fs::no_check), work)))
                tmp.push_back(ph);
            return tmp;
        }
    }
    else
    {
        const std::string ptn(pattern, 0, slash);
        const std::string rest_ptn(pattern, slash+1);

        if (!::contains_wildcard(ptn))
        {
            const std::string& ph = ::path_join_nt(dir, ptn);
            if (fs::is_directory(fs::complete(fs::path(ph,fs::no_check), work)))
                return ::glob_recursive(work, ph, rest_ptn);
            else
                return std::vector<std::string>();
        }

        const std::vector<std::string>& dirs =
            ::glob_impl(work, dir, ptn, true);

        std::vector<std::string> result;
        for (std::size_t i = 0, size = dirs.size(); i < size; ++i)
        {
            const std::vector<std::string>& tmp =
                ::glob_recursive(work, dirs[i], rest_ptn);

            result.insert(result.end(), tmp.begin(), tmp.end());
        }
        return result;
    }
}

inline std::vector<std::string>
glob_recursive(const boost::filesystem::path& work, const std::string& pattern)
{
    if ((pattern.size() >= 3) && (pattern[1] == ':'))
        return ::glob_recursive(work, pattern.substr(0, 3), pattern.substr(3));
    else
        return ::glob_recursive(work, "", pattern);
}

#endif // IMPL_GLOB_HPP
