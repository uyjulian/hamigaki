// path.hpp: utility functions for basic_path

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_PATH_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_PATH_HPP

#include <boost/filesystem/path.hpp>
#include <boost/version.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Path>
inline Path remove_root_name(const Path& ph)
{
    if (ph.has_root_name())
        return ph.root_directory() / ph.relative_path();
    else
        return ph;
}

template<class Path>
inline bool has_parent_path(const Path& ph)
{
#if BOOST_VERSION < 103600
    return ph.has_branch_path();
#else
    return ph.has_parent_path();
#endif
}

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_PATH_HPP
