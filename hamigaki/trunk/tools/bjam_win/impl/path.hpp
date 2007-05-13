// path.hpp: path module

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_PATH_HPP
#define IMPL_PATH_HPP

#include "./glob.hpp"
#include "./normalize_path.hpp"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <algorithm>
#include <stdexcept>

namespace path
{

namespace impl
{

inline void split(std::vector<std::string>& result, const std::string& ph)
{
    namespace algo = boost::algorithm;
    algo::split(result, ph, algo::is_any_of("/"));
}

inline std::string join2_impl(const std::string& ph1, const std::string& ph2)
{
    std::vector<std::string> parts;
    if (!ph1.empty())
        impl::split(parts, ph1);
    else
        parts.push_back("/");

    std::vector<std::string> tmp;
    impl::split(tmp, ph2);
    parts.insert(parts.end(), tmp.begin(), tmp.end());

    return ::normalize_path(parts);
}

inline void glob_recursive_append(
    std::vector<std::string>& result,
    const boost::filesystem::path& work,
    const std::vector<std::string>& patterns)
{
    for (std::size_t i = 0, size = patterns.size(); i < size; ++i)
    {
        const std::vector<std::string>& tmp =
            ::glob_recursive(work, patterns[i]);
        result.insert(result.end(), tmp.begin(), tmp.end());
    }
}

} // namespace impl


inline std::string make(const std::string& ph)
{
    std::string result = ph;
    boost::algorithm::replace_all(result, "\\", "/");

    if (!result.empty() && (*result.rbegin() == '/'))
        result.resize(result.size()-1);

    if ((result.size() >= 2) && (result[1] == ':'))
        result.insert(0, 1u, '/');

    return result;
}

inline std::string native(const std::string& ph)
{
    std::string::size_type start = 0;
    if (!ph.empty() && (ph[0] == '/'))
        start = 1;

    std::string result(ph, start);
    boost::algorithm::replace_all(result, "/", "\\");
    return result;
}

inline bool is_rooted(const std::string& ph)
{
    return !ph.empty() && (ph[0] == '/');
}

inline std::string join2(const std::string& ph1, const std::string& ph2)
{
    if (ph1.empty())
        return ph2;
    else
    {
        if (path::is_rooted(ph2))
            throw std::invalid_argument("only first element may be rooted");

        return impl::join2_impl(ph1, ph2);
    }
}

inline std::string root(const std::string& ph, const std::string& root)
{
    if (path::is_rooted(ph))
        return ph;
    else
        return path::join2(root, ph);
}

inline std::vector<std::string>
glob(
    const boost::filesystem::path& work,
    const std::vector<std::string>& dirs,
    const std::vector<std::string>& patterns,
    const std::vector<std::string>& exclude_patterns)
{
    std::vector<std::string> ptns;
    std::vector<std::string> ex_ptns;

    for (std::size_t j = 0; j < dirs.size(); ++j)
    {
        for (std::size_t i = 0; i < patterns.size(); ++i)
            ptns.push_back(path::native(path::root(patterns[i], dirs[j])));

        for (std::size_t i = 0; i < exclude_patterns.size(); ++i)
        {
            ex_ptns.push_back(
                path::native(path::root(exclude_patterns[i], dirs[j])));
        }
    }

    std::vector<std::string> inc;
    impl::glob_recursive_append(inc, work, ptns);

    std::vector<std::string> exc;
    impl::glob_recursive_append(exc, work, ex_ptns);

    // TODO: don't sort
    std::sort(inc.begin(), inc.end());
    std::sort(exc.begin(), exc.end());

    std::vector<std::string> result;
    std::set_difference(
        inc.begin(), inc.end(),
        exc.begin(), exc.end(),
        std::back_inserter(result)
    );

    std::transform(result.begin(), result.end(), result.begin(), &path::make);

    return result;
}

} // namespace path

#endif // IMPL_PATH_HPP
