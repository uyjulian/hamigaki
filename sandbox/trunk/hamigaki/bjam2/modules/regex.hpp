// regex.hpp: bjam regex module

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_MODULES_REGEX_HPP
#define HAMIGAKI_BJAM2_MODULES_REGEX_HPP

#include <hamigaki/bjam2/bjam_config.hpp>
#include <hamigaki/bjam2/util/list.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam2 {

class context;

namespace modules
{

namespace regex
{

HAMIGAKI_BJAM2_DECL string_list transform(context& ctx);

} // namespace regex

HAMIGAKI_BJAM2_DECL void set_regex_rules(context& ctx);

} // namespace modules

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_MODULES_REGEX_HPP
