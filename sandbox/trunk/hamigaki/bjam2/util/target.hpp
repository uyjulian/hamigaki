// target.hpp: bjam target

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_TARGET_HPP
#define HAMIGAKI_BJAM2_UTIL_TARGET_HPP

#include <hamigaki/bjam2/util/variable_table.hpp>
#include <set>

namespace hamigaki { namespace bjam2 {

struct target
{
    static const unsigned temporary     = 0x0001;
    static const unsigned no_care       = 0x0002;
    static const unsigned not_file      = 0x0004;
    static const unsigned force_update  = 0x0008;
    static const unsigned leaves        = 0x0010;
    static const unsigned no_update     = 0x0020;
    static const unsigned rm_old        = 0x0040;
    static const unsigned fail_expected = 0x0080;
    static const unsigned is_file       = 0x0100;

    variable_table variables;
    std::set<std::string> depended_targets;
    std::set<std::string> included_targets;
    std::set<std::string> rebuilt_targets;
    unsigned flags;

    target() : flags(0)
    {
    }
};

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_TARGET_HPP
