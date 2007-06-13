// glob.cpp: glob for bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/util/glob.hpp>
#include <hamigaki/bjam/util/pattern.hpp>
#include <hamigaki/bjam/util/path.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/version.hpp>

namespace algo = boost::algorithm;
namespace fs = boost::filesystem;

namespace hamigaki { namespace bjam {

namespace
{

inline bool contains_wildcard(const std::string& s)
{
    return s.find_first_of("?*[]") != std::string::npos;
}

string_list glob_impl(
    const boost::filesystem::path& work, const std::string& dir,
    const std::string& pattern, bool case_insensitive, bool dir_only)
{
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    case_insensitive = true;
#endif

    string_list result;

    fs::path ph(dir, fs::native);
    ph = fs::complete(ph, work);

    path_components compo;
    compo.dir = dir;

    fs::directory_iterator it(ph);
    fs::directory_iterator end;

    std::string ptn = pattern;
    if (case_insensitive)
        algo::to_lower(ptn);

    for (; it != end; ++it)
    {
#if BOOST_VERSION < 103400
        if (dir_only && !is_directory(*it))
            continue;
        std::string leaf = it->leaf();
#else
        if (dir_only && !is_directory(it->status()))
            continue;
        std::string leaf = it->path().leaf();
#endif
        std::string s = leaf;
        if (case_insensitive)
            algo::to_lower(s);

        if (pattern_match(ptn, s))
        {
            compo.base = leaf;
            result += make_path(compo);
        }
    }

    return result;
}

string_list
glob_recursive_impl(
    const boost::filesystem::path& work,
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
            return glob_impl(work, dir, pattern, false, false);
        else
        {
            path_components compo;
            compo.dir = dir;
            compo.base = pattern;

            const std::string& ph = make_path(compo);

            string_list tmp;
            if (fs::exists(fs::complete(fs::path(ph, fs::no_check), work)))
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
            fs::path ph(new_dir, fs::no_check);
            ph = fs::complete(ph, work);

#if BOOST_VERSION < 103400
            if (fs::exists(ph) && fs::is_directory(ph))
#else
            if (fs::is_directory(ph))
#endif
            {
                return glob_recursive_impl(work, new_dir, rest_ptn);
            }
            else
                return string_list();
        }

        const string_list& dirs = glob_impl(work, dir, ptn, false, true);

        string_list result;
        for (std::size_t i = 0, size = dirs.size(); i < size; ++i)
            result += glob_recursive_impl(work, dirs[i], rest_ptn);
        return result;
    }
}

} // namespace

HAMIGAKI_BJAM_DECL string_list glob(
    const std::string& work, const std::string& dir,
    const std::string& pattern, bool case_insensitive)
{
    return glob_impl(
        fs::path(work, fs::native), dir, pattern, case_insensitive, false);
}

HAMIGAKI_BJAM_DECL string_list
glob_recursive(const std::string& work, const std::string& pattern)
{
    fs::path work_ph(work, fs::native);

    if ((pattern.size() >= 3) && (pattern[1] == ':'))
    {
        return glob_recursive_impl(
            work_ph, pattern.substr(0, 3), pattern.substr(3));
    }
#if defined(BOOST_WINDOWS)
    else if ((pattern[0] == '/') || (pattern[0] == '\\'))
#else
    else if (pattern[0] == '/')
#endif
    {
        return glob_recursive_impl(
            work_ph, pattern.substr(0, 1), pattern.substr(1));
    }
    else
        return glob_recursive_impl(work_ph, "", pattern);
}

} } // End namespaces bjam, hamigaki.
