// glob.hpp: glob for bjam

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_GLOB_HPP
#define HAMIGAKI_BJAM2_UTIL_GLOB_HPP

#include <hamigaki/bjam2/bjam_config.hpp>
#include <hamigaki/bjam2/util/file_status_cache.hpp>
#include <hamigaki/bjam2/util/list.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam2 {

HAMIGAKI_BJAM2_DECL string_list glob(
    file_status_cache& cache, const std::string& dir,
    const std::string& pattern, bool case_insensitive = false);

HAMIGAKI_BJAM2_DECL string_list
glob_recursive(file_status_cache& cache, const std::string& pattern);

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_UTIL_GLOB_HPP
