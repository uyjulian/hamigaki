// glob.cpp: glob for bjam

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM2_SOURCE
#include <hamigaki/bjam2/util/glob.hpp>
#include <hamigaki/bjam2/util/pattern.hpp>
#include <hamigaki/bjam2/util/path.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/version.hpp>

namespace algo = boost::algorithm;
namespace fs = boost::filesystem;

namespace hamigaki { namespace bjam2 {

namespace
{

inline bool contains_wildcard(const std::string& s)
{
    return s.find_first_of("?*[]") != std::string::npos;
}

string_list glob_impl(
    file_status_cache& cache, const std::string& dir,
    const std::string& pattern, bool case_insensitive, bool dir_only)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    case_insensitive = true;
#endif

    string_list result;

    const std::vector<std::string>& entries = cache.directory_entries(dir);
    if (entries.empty())
        return result;

    path_components compo;
    compo.dir = dir;

    typedef std::vector<std::string>::const_iterator const_iterator;
    const_iterator it = entries.begin();
    const_iterator end = entries.end();

    std::string ptn = pattern;
    if (case_insensitive)
        algo::to_lower(ptn);

    for (; it != end; ++it)
    {
        compo.base = *it;
        const std::string& ph = make_path(compo);

        if (dir_only && !is_directory(cache.status(ph)))
            continue;

        std::string s = compo.base;
        if (case_insensitive)
            algo::to_lower(s);

        if (pattern_match(ptn, s))
            result += ph;
    }

    return result;
}

string_list
glob_recursive_impl(
    file_status_cache& cache,
    const std::string& dir, const std::string& pattern)
{
#if defined(BOOST_WINDOWS)
    std::string::size_type slash = pattern.find_first_of("/\\");
#else
    std::string::size_type slash = pattern.find("/");
#endif
    if (slash == std::string::npos)
    {
        if (contains_wildcard(pattern))
            return glob_impl(cache, dir, pattern, false, false);
        else
        {
            path_components compo;
            compo.dir = dir;
            compo.base = pattern;

            const std::string& ph = make_path(compo);

            string_list tmp;
            if (fs::exists(cache.status(ph)))
                tmp.push_back(ph);
            return tmp;
        }
    }
    else
    {
        const std::string ptn(pattern, 0, slash);
        const std::string rest_ptn(pattern, slash+1);

        if (!contains_wildcard(ptn))
        {
            path_components compo;
            compo.dir = dir;
            compo.base = ptn;

            const std::string& new_dir = make_path(compo);
            if (fs::is_directory(cache.status(new_dir)))
                return glob_recursive_impl(cache, new_dir, rest_ptn);
            else
                return string_list();
        }

        const string_list& dirs = glob_impl(cache, dir, ptn, false, true);

        string_list result;
        for (std::size_t i = 0, size = dirs.size(); i < size; ++i)
            result += glob_recursive_impl(cache, dirs[i], rest_ptn);
        return result;
    }
}

} // namespace

HAMIGAKI_BJAM2_DECL string_list glob(
    file_status_cache& cache, const std::string& dir,
    const std::string& pattern, bool case_insensitive)
{
    return glob_impl(cache, dir, pattern, case_insensitive, false);
}

HAMIGAKI_BJAM2_DECL string_list
glob_recursive(file_status_cache& cache, const std::string& pattern)
{
    if ((pattern.size() >= 3) && (pattern[1] == ':'))
    {
        return glob_recursive_impl(
            cache, pattern.substr(0, 3), pattern.substr(3));
    }
#if defined(BOOST_WINDOWS)
    else if ((pattern[0] == '/') || (pattern[0] == '\\'))
#else
    else if (pattern[0] == '/')
#endif
    {
        return glob_recursive_impl(
            cache, pattern.substr(0, 1), pattern.substr(1));
    }
    else
        return glob_recursive_impl(cache, "", pattern);
}

} } // End namespaces bjam2, hamigaki.
