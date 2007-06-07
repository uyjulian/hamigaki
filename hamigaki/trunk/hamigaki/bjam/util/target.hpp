// target.hpp: bjam target

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_TARGET_HPP
#define HAMIGAKI_BJAM_UTIL_TARGET_HPP

#include <hamigaki/bjam/util/variable_table.hpp>
#include <set>

namespace hamigaki { namespace bjam {

struct target
{
    variable_table variables;
    std::set<std::string> depended_targets;
    std::set<std::string> included_targets;
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_TARGET_HPP
