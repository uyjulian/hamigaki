// glob.hpp: glob for bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_GLOB_HPP
#define HAMIGAKI_BJAM_UTIL_GLOB_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/util/list.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

HAMIGAKI_BJAM_DECL string_list glob(
    const std::string& work, const std::string& dir,
    const std::string& pattern, bool case_insensitive = false);

HAMIGAKI_BJAM_DECL string_list
glob_recursive(const std::string& work, const std::string& pattern);

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_UTIL_GLOB_HPP
