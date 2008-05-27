// search.hpp: search the target file

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_SEARCH_HPP
#define HAMIGAKI_BJAM2_UTIL_SEARCH_HPP

#include <hamigaki/bjam2/bjam_config.hpp>
#include <string>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam2 {

class context;

HAMIGAKI_BJAM2_DECL
void call_bind_rule(
    context& ctx, const std::string& name, const std::string& filename);

HAMIGAKI_BJAM2_DECL
std::string search_target(context& ctx, const std::string& name);

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_UTIL_SEARCH_HPP
